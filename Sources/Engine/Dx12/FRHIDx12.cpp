/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"

#if COMPILE_WITH_RHI_DX12
#include "FRHIDx12Conversion.h"
#include "TDeviceWin32.h"
#include "FFrameResourcesDx12.h"
#include "FMeshBufferDx12.h"
#include "FTextureDx12.h"
#include "FPipelineDx12.h"
#include "FUniformBufferDx12.h"
#include "FRenderTargetDx12.h"
#include "FShaderDx12.h"
#include <DirectXColors.h>

// link libraries
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")

namespace tix
{
#if defined (TIX_DEBUG)
#	define DX_SETNAME(Resource, Name) SetResourceName(Resource, Name)
#else
#	define DX_SETNAME(Resource, Name)
#endif

	FRHIDx12::FRHIDx12()
		: FRHI(ERHI_DX12)
		, CurrentFrame(0)
		, NumBarriersToFlush(0)
	{
		// Create frame resource holders
		for (int32 i = 0; i < FRHIConfig::FrameBufferNum; ++i)
		{
			ResHolders[i] = ti_new FFrameResourcesDx12;
			FrameResources[i] = ResHolders[i];
		}
	}

	FRHIDx12::~FRHIDx12()
	{
		// delete frame resource holders
		for (int32 i = 0; i < FRHIConfig::FrameBufferNum; ++i)
		{
			ti_delete ResHolders[i];
			ResHolders[i] = nullptr;
			FrameResources[i] = nullptr;
		}
	}

	void FRHIDx12::InitRHI()
	{
#if defined(TIX_DEBUG)
		// If the project is in a debug build, enable debugging via SDK Layers.
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
			}
		}
#endif
		// Create D3D12 Device
		HRESULT hr;
		VALIDATE_HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&DxgiFactory)));

		ComPtr<IDXGIAdapter1> adapter;
		GetHardwareAdapter(&adapter);

		// Create the Direct3D 12 API device object
		hr = D3D12CreateDevice(
			adapter.Get(),					// The hardware adapter.
			D3D_FEATURE_LEVEL_11_0,			// Minimum feature level this app can support.
			IID_PPV_ARGS(&D3dDevice)		// Returns the Direct3D device created.
		);

#if defined(TIX_DEBUG)
		if (FAILED(hr))
		{
			// If the initialization fails, fall back to the WARP device.
			// For more information on WARP, see: 
			// https://go.microsoft.com/fwlink/?LinkId=286690

			ComPtr<IDXGIAdapter> warpAdapter;
			DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

			hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&D3dDevice));
		}
#endif
		VALIDATE_HRESULT(hr);

		// Prevent the GPU from over-clocking or under-clocking to get consistent timings
		//if (DeveloperModeEnabled)
		//	D3dDevice->SetStablePowerState(TRUE);

		// Create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		VALIDATE_HRESULT(D3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue)));
		CommandQueue->SetName(L"CommandQueue");

		// Create descriptor heaps for render target views and depth stencil views.
		DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].Create(this, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].Create(this, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		// Create command allocator and command list.
		for (uint32 n = 0; n < FRHIConfig::FrameBufferNum; n++)
		{
			VALIDATE_HRESULT(D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocators[n])));
		}

		// Create synchronization objects.
		hr = D3dDevice->CreateFence(FenceValues[CurrentFrame], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
		VALIDATE_HRESULT(hr);
		FenceValues[CurrentFrame]++;

		FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (FenceEvent == nullptr)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			TI_ASSERT(0);
		}

		CreateWindowsSizeDependentResources();

		// Describe and create a shader resource view (SRV) heap for the texture.
		DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Create(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Create Default Command list
		VALIDATE_HRESULT(D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocators[CurrentFrame].Get(), nullptr, IID_PPV_ARGS(&CommandList)));
		VALIDATE_HRESULT(CommandList->Close());

		_LOG(Log, "  RHI DirectX 12 inited.\n");
	}

	// This method acquires the first available hardware adapter that supports Direct3D 12.
	// If no such adapter can be found, *ppAdapter will be set to nullptr.
	void FRHIDx12::GetHardwareAdapter(IDXGIAdapter1** ppAdapter)
	{
		ComPtr<IDXGIAdapter1> adapter;
		*ppAdapter = nullptr;

		for (uint32 adapterIndex = 0; DXGI_ERROR_NOT_FOUND != DxgiFactory->EnumAdapters1(adapterIndex, &adapter); adapterIndex++)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				continue;
			}

			// Check to see if the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				char AdapterName[128];
				size_t Converted;
				wcstombs_s(&Converted, AdapterName, 128, desc.Description, 128);
				_LOG(Log, "D3D12-capable hardware found:  %s (%u MB)\n", AdapterName, desc.DedicatedVideoMemory >> 20);
				break;
			}
		}

		*ppAdapter = adapter.Detach();
	}

	void FRHIDx12::CreateWindowsSizeDependentResources()
	{
		// Clear the previous window size specific content and update the tracked fence values.
		for (uint32 n = 0; n < FRHIConfig::FrameBufferNum; n++)
		{
			BackBufferRTs[n] = nullptr;
			FenceValues[n] = FenceValues[CurrentFrame];
		}

		// Use windows' device size directly.
		vector2di WindowSize = TEngine::Get()->GetDevice()->GetDeviceSize();

		// The width and height of the swap chain must be based on the window's
		// natively-oriented width and height. If the window is not in the native
		// orientation, the dimensions must be reversed.
		DXGI_MODE_ROTATION displayRotation = DXGI_MODE_ROTATION_IDENTITY;

		uint32 BackBufferWidth = lround(WindowSize.X);
		uint32 BackBufferHeight = lround(WindowSize.Y);

		HRESULT hr;

		const DXGI_FORMAT BackBufferFormat = GetDxPixelFormat(FRHIConfig::DefaultBackBufferFormat);
		const DXGI_FORMAT DepthBufferFormat = GetDxPixelFormat(FRHIConfig::DefaultDepthBufferFormat);

		if (SwapChain != nullptr)
		{
			// If the swap chain already exists, resize it.
			HRESULT hr = SwapChain->ResizeBuffers(FRHIConfig::FrameBufferNum, BackBufferWidth, BackBufferHeight, BackBufferFormat, 0);

			if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
			{
				// If the device was removed for any reason, a new device and swap chain will need to be created.

				// Do not continue execution of this method. DeviceResources will be destroyed and re-created.
				return;
			}
			else
			{
				VALIDATE_HRESULT(hr);
			}
		}
		else
		{
			// Otherwise, create a new one using the same adapter as the existing Direct3D device.
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

			swapChainDesc.Width = BackBufferWidth;						// Match the size of the window.
			swapChainDesc.Height = BackBufferHeight;
			swapChainDesc.Format = BackBufferFormat;
			swapChainDesc.Stereo = false;
			swapChainDesc.SampleDesc.Count = 1;							// Don't use multi-sampling.
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = FRHIConfig::FrameBufferNum;			// Use triple-buffering to minimize latency.
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// All Windows Universal apps must use _FLIP_ SwapEffects.
			swapChainDesc.Flags = 0;
			swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
			swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

			ComPtr<IDXGISwapChain1> swapChain;

			TDeviceWin32* DeviceWin32 = dynamic_cast<TDeviceWin32*>(TEngine::Get()->GetDevice());
			TI_ASSERT(DeviceWin32);
			HWND HWnd = DeviceWin32->GetWnd();

			hr = DxgiFactory->CreateSwapChainForHwnd(
				CommandQueue.Get(),								// Swap chains need a reference to the command queue in DirectX 12.
				HWnd,
				&swapChainDesc,
				nullptr,
				nullptr,
				&swapChain
			);
			VALIDATE_HRESULT(hr);

			hr = swapChain.As(&SwapChain);
			VALIDATE_HRESULT(hr);
		}

		hr = SwapChain->SetRotation(displayRotation);
		VALIDATE_HRESULT(hr);

		// Create render target views of the swap chain back buffer.
		{
			CurrentFrame = SwapChain->GetCurrentBackBufferIndex();

			BackBufferDescriptorTable = RenderResourceHeap[EHT_RENDERTARGET].AllocateTable(FRHIConfig::FrameBufferNum);
			for (uint32 n = 0; n < FRHIConfig::FrameBufferNum; n++)
			{
				BackBufferDescriptors[n] = GetCpuDescriptorHandle(EHT_RENDERTARGET, BackBufferDescriptorTable->GetIndexAt(n));
				VALIDATE_HRESULT(SwapChain->GetBuffer(n, IID_PPV_ARGS(&BackBufferRTs[n])));
				D3dDevice->CreateRenderTargetView(BackBufferRTs[n].Get(), nullptr, BackBufferDescriptors[n]);

				WCHAR name[32];
				if (swprintf_s(name, L"BackBufferRTs[%u]", n) > 0)
				{
					BackBufferRTs[n].Get()->SetName(name);
				}
			}
		}

		// Create a depth stencil and view.
		{
			D3D12_HEAP_PROPERTIES depthHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

			D3D12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DepthBufferFormat, BackBufferWidth, BackBufferHeight, 1, 1);
			depthResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			CD3DX12_CLEAR_VALUE depthOptimizedClearValue(DepthBufferFormat, 1.0f, 0);

			hr = D3dDevice->CreateCommittedResource(
				&depthHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&depthResourceDesc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&depthOptimizedClearValue,
				IID_PPV_ARGS(&DepthStencil)
			);
			VALIDATE_HRESULT(hr);

			DepthStencil->SetName(L"DepthStencil");

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = DepthBufferFormat;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

			DepthStencilDescriptorTable = RenderResourceHeap[EHT_DEPTHSTENCIL].AllocateTable(1);
			DepthStencilDescriptor = GetCpuDescriptorHandle(EHT_DEPTHSTENCIL, DepthStencilDescriptorTable->GetStartIndex());
			D3dDevice->CreateDepthStencilView(DepthStencil.Get(), &dsvDesc, DepthStencilDescriptor);
		}

		// Init default sampler
		{
			DefaultSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			DefaultSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			DefaultSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			DefaultSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			DefaultSampler.MipLODBias = 0;
			DefaultSampler.MaxAnisotropy = 0;
			DefaultSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			DefaultSampler.MinLOD = 0.0f;
			DefaultSampler.MaxLOD = D3D12_FLOAT32_MAX;
		}
		
		// Set the 3D rendering viewport to target the entire window.
		FRHI::SetViewport(FViewport(0, 0, BackBufferWidth, BackBufferHeight));
	}

	void FRHIDx12::BeginFrame()
	{
		// Reset command allocator
		VALIDATE_HRESULT(CommandAllocators[CurrentFrame]->Reset());
		TI_ASSERT(NumBarriersToFlush == 0);

		// Reset command list
		VALIDATE_HRESULT(CommandList->Reset(CommandAllocators[CurrentFrame].Get(), nullptr));

		// Set the graphics root signature and descriptor heaps to be used by this frame.
		ID3D12DescriptorHeap* ppHeaps[] = { DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetHeap() };
		CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		// Indicate this resource will be in use as a render target.
		Transition(BackBufferRTs[CurrentFrame].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		FlushResourceBarriers(CommandList.Get());

		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = BackBufferDescriptors[CurrentFrame];
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilDescriptor;
		CommandList->ClearRenderTargetView(renderTargetView, DirectX::Colors::CornflowerBlue, 0, nullptr);
		CommandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		CommandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);

		// Set the viewport and scissor rectangle.
		D3D12_VIEWPORT ViewportDx = { float(Viewport.Left), float(Viewport.Top), float(Viewport.Width), float(Viewport.Height), 0.f, 1.f };
		CommandList->RSSetViewports(1, &ViewportDx);
		D3D12_RECT ScissorRect = { Viewport.Left, Viewport.Top, Viewport.Width, Viewport.Height };
		CommandList->RSSetScissorRects(1, &ScissorRect);
	}

	void FRHIDx12::EndFrame()
	{
		// Indicate that the render target will now be used to present when the command list is done executing.
		Transition(BackBufferRTs[CurrentFrame].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		FlushResourceBarriers(CommandList.Get());

		// Execute the command list.
		VALIDATE_HRESULT(CommandList->Close());
		ID3D12CommandList* ppCommandLists[] = { CommandList.Get() };
		CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// The first argument instructs DXGI to block until VSync, putting the application
		// to sleep until the next VSync. This ensures we don't waste any cycles rendering
		// frames that will never be displayed to the screen.
		HRESULT hr = SwapChain->Present(1, 0);

		// If the device was removed either by a disconnection or a driver upgrade, we 
		// must recreate all device resources.
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
		}
		else
		{
			VALIDATE_HRESULT(hr);

			MoveToNextFrame();
		}

		// Reset current bound resources
		CurrentBoundResource.Reset();
	}

	FTexturePtr FRHIDx12::CreateTexture()
	{
		return ti_new FTextureDx12();
	}

	FTexturePtr FRHIDx12::CreateTexture(const TTextureDesc& Desc)
	{
		return ti_new FTextureDx12(Desc);
	}

	FUniformBufferPtr FRHIDx12::CreateUniformBuffer(uint32 InStructSize)
	{
		return ti_new FUniformBufferDx12(InStructSize);
	}

	FMeshBufferPtr FRHIDx12::CreateMeshBuffer()
	{
		return ti_new FMeshBufferDx12();
	}

	FPipelinePtr FRHIDx12::CreatePipeline()
	{
		return ti_new FPipelineDx12();
	}

	FRenderTargetPtr FRHIDx12::CreateRenderTarget(int32 W, int32 H)
	{
		return ti_new FRenderTargetDx12(W, H);
	}

	FShaderBindingPtr FRHIDx12::CreateShaderBinding(uint32 NumBindings)
	{
		return ti_new FRootSignatureDx12(NumBindings, FRHIConfig::StaticSamplerNum);
	}

	FShaderPtr FRHIDx12::CreateShader(const TShaderNames& InNames)
	{
		return ti_new FShaderDx12(InNames);
	}

	// Wait for pending GPU work to complete.
	void FRHIDx12::WaitingForGpu()
	{
		// Schedule a Signal command in the queue.
		VALIDATE_HRESULT(CommandQueue->Signal(Fence.Get(), FenceValues[CurrentFrame]));

		// Wait until the fence has been crossed.
		VALIDATE_HRESULT(Fence->SetEventOnCompletion(FenceValues[CurrentFrame], FenceEvent));
		WaitForSingleObjectEx(FenceEvent, INFINITE, FALSE);

		// Increment the fence value for the current frame.
		FenceValues[CurrentFrame]++;
	}

	// Prepare to render the next frame.
	void FRHIDx12::MoveToNextFrame()
	{
		// Remember the frame index to release holded reference.
		const int32 FrameToRelease = CurrentFrame;

		// Schedule a Signal command in the queue.
		const uint64 currentFenceValue = FenceValues[CurrentFrame];
		VALIDATE_HRESULT(CommandQueue->Signal(Fence.Get(), currentFenceValue));

		// Advance the frame index.
		CurrentFrame = SwapChain->GetCurrentBackBufferIndex();

		// Check to see if the next frame is ready to start.
		if (Fence->GetCompletedValue() < FenceValues[CurrentFrame])
		{
			VALIDATE_HRESULT(Fence->SetEventOnCompletion(FenceValues[CurrentFrame], FenceEvent));
			WaitForSingleObjectEx(FenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		FenceValues[CurrentFrame] = currentFenceValue + 1;

		// Release resources references
		FrameResources[FrameToRelease]->RemoveAllReferences();
	}

	void FRHIDx12::HoldResourceReference(FRenderResourcePtr InResource)
	{
		ResHolders[CurrentFrame]->HoldReference(InResource);
	}

	void FRHIDx12::HoldResourceReference(ComPtr<ID3D12Resource> InDxResource)
	{
		ResHolders[CurrentFrame]->HoldDxReference(InDxResource);
	}

	void FRHIDx12::SetResourceName(ID3D12Resource* InDxResource, const TString& InName)
	{
		TWString WName = FromString(InName);
		InDxResource->SetName(WName.c_str());
	}

	void FRHIDx12::SetResourceName(ID3D12PipelineState* InDxResource, const TString& InName)
	{
		TWString WName = FromString(InName);
		InDxResource->SetName(WName.c_str());
	}

	//------------------------------------------------------------------------------------------------
	// All arrays must be populated (e.g. by calling GetCopyableFootprints)
	uint64 FRHIDx12::UpdateSubresources(
		_In_ ID3D12GraphicsCommandList* pCmdList,
		_In_ ID3D12Resource* pDestinationResource,
		_In_ ID3D12Resource* pIntermediate,
		_In_range_(0, D3D12_REQ_SUBRESOURCES) uint32 FirstSubresource,
		_In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) uint32 NumSubresources,
		uint64 RequiredSize,
		_In_reads_(NumSubresources) const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
		_In_reads_(NumSubresources) const uint32* pNumRows,
		_In_reads_(NumSubresources) const uint64* pRowSizesInBytes,
		_In_reads_(NumSubresources) const D3D12_SUBRESOURCE_DATA* pSrcData)
	{
		// Minor validation
		D3D12_RESOURCE_DESC IntermediateDesc = pIntermediate->GetDesc();
		D3D12_RESOURCE_DESC DestinationDesc = pDestinationResource->GetDesc();
		if (IntermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
			IntermediateDesc.Width < RequiredSize + pLayouts[0].Offset ||
			RequiredSize >(SIZE_T) - 1 ||
			(DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER &&
			(FirstSubresource != 0 || NumSubresources != 1)))
		{
			return 0;
		}

		uint8* pData;
		HRESULT hr = pIntermediate->Map(0, nullptr, reinterpret_cast<void**>(&pData));
		if (FAILED(hr))
		{
			return 0;
		}

		for (uint32 i = 0; i < NumSubresources; ++i)
		{
			if (pRowSizesInBytes[i] > (SIZE_T)-1) return 0;
			D3D12_MEMCPY_DEST DestData = { pData + pLayouts[i].Offset, pLayouts[i].Footprint.RowPitch, pLayouts[i].Footprint.RowPitch * pNumRows[i] };
			MemcpySubresource(&DestData, &pSrcData[i], (SIZE_T)pRowSizesInBytes[i], pNumRows[i], pLayouts[i].Footprint.Depth);
		}
		pIntermediate->Unmap(0, nullptr);

		if (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			CD3DX12_BOX SrcBox(uint32(pLayouts[0].Offset), uint32(pLayouts[0].Offset + pLayouts[0].Footprint.Width));
			pCmdList->CopyBufferRegion(
				pDestinationResource, 0, pIntermediate, pLayouts[0].Offset, pLayouts[0].Footprint.Width);
		}
		else
		{
			for (uint32 i = 0; i < NumSubresources; ++i)
			{
				CD3DX12_TEXTURE_COPY_LOCATION Dst(pDestinationResource, i + FirstSubresource);
				CD3DX12_TEXTURE_COPY_LOCATION Src(pIntermediate, pLayouts[i]);
				pCmdList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
			}
		}
		return RequiredSize;
	}

	//------------------------------------------------------------------------------------------------
	// Heap-allocating UpdateSubresources implementation
	uint64 FRHIDx12::UpdateSubresources(
		_In_ ID3D12GraphicsCommandList* pCmdList,
		_In_ ID3D12Resource* pDestinationResource,
		_In_ ID3D12Resource* pIntermediate,
		uint64 IntermediateOffset,
		_In_range_(0, D3D12_REQ_SUBRESOURCES) uint32 FirstSubresource,
		_In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) uint32 NumSubresources,
		_In_reads_(NumSubresources) D3D12_SUBRESOURCE_DATA* pSrcData)
	{
		uint64 RequiredSize = 0;
		uint64 MemToAlloc = static_cast<uint64>(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(uint32) + sizeof(uint64)) * NumSubresources;
		if (MemToAlloc > SIZE_MAX)
		{
			return 0;
		}
		void* pMem = HeapAlloc(GetProcessHeap(), 0, static_cast<SIZE_T>(MemToAlloc));
		if (pMem == nullptr)
		{
			return 0;
		}
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(pMem);
		uint64* pRowSizesInBytes = reinterpret_cast<uint64*>(pLayouts + NumSubresources);
		uint32* pNumRows = reinterpret_cast<uint32*>(pRowSizesInBytes + NumSubresources);

		D3D12_RESOURCE_DESC Desc = pDestinationResource->GetDesc();
		ID3D12Device* pDevice;
		pDestinationResource->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
		pDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, IntermediateOffset, pLayouts, pNumRows, pRowSizesInBytes, &RequiredSize);
		pDevice->Release();

		uint64 Result = UpdateSubresources(pCmdList, pDestinationResource, pIntermediate, FirstSubresource, NumSubresources, RequiredSize, pLayouts, pNumRows, pRowSizesInBytes, pSrcData);
		HeapFree(GetProcessHeap(), 0, pMem);
		return Result;
	}

	void FRHIDx12::Transition(
		FGPUResourceDx12* GPUResource,
		D3D12_RESOURCE_STATES stateAfter,
		uint32 subresource,
		D3D12_RESOURCE_BARRIER_FLAGS flags)
	{
		if (GPUResource->GetCurrentState() != stateAfter)
		{
			Transition(GPUResource->GetResource(), GPUResource->UsageState, stateAfter, subresource, flags);
			GPUResource->UsageState = stateAfter;
		}
	}

	void FRHIDx12::Transition(
		_In_ ID3D12Resource* pResource,
		D3D12_RESOURCE_STATES stateBefore,
		D3D12_RESOURCE_STATES stateAfter,
		uint32 subresource,
		D3D12_RESOURCE_BARRIER_FLAGS flags)
	{
		D3D12_RESOURCE_BARRIER& barrier = ResourceBarrierBuffers[NumBarriersToFlush++];
		TI_ASSERT(NumBarriersToFlush <= MaxResourceBarrierBuffers);
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = flags;
		barrier.Transition.pResource = pResource;
		barrier.Transition.StateBefore = stateBefore;
		barrier.Transition.StateAfter = stateAfter;
		barrier.Transition.Subresource = subresource;
	}

	void FRHIDx12::FlushResourceBarriers(
		_In_ ID3D12GraphicsCommandList* pCmdList)
	{
		if (NumBarriersToFlush > 0)
		{
			pCmdList->ResourceBarrier(NumBarriersToFlush, ResourceBarrierBuffers);
			NumBarriersToFlush = 0;
		}
	}

	bool FRHIDx12::UpdateHardwareResource(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData)
	{
#if defined (TIX_DEBUG)
		MeshBuffer->SetResourceName(InMeshData->GetResourceName());
#endif
		FMeshBufferDx12 * MBDx12 = static_cast<FMeshBufferDx12*>(MeshBuffer.get());

		// Create the vertex buffer resource in the GPU's default heap and copy vertex data into it using the upload heap.
		// The upload resource must not be released until after the GPU has finished using it.
		ComPtr<ID3D12Resource> VertexBufferUpload;

		const int32 BufferSize = InMeshData->GetVerticesCount() * InMeshData->GetStride();
		CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(BufferSize);
		VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&vertexBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&MBDx12->VertexBuffer)));

		CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&vertexBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&VertexBufferUpload)));

		DX_SETNAME(MBDx12->VertexBuffer.Get(), MeshBuffer->GetResourceName() + "-VB");

		// Upload the vertex buffer to the GPU.
		{
			D3D12_SUBRESOURCE_DATA VertexData = {};
			VertexData.pData = reinterpret_cast<const uint8*>(InMeshData->GetVSData());
			VertexData.RowPitch = BufferSize;
			VertexData.SlicePitch = VertexData.RowPitch;

			UpdateSubresources(CommandList.Get(), MBDx12->VertexBuffer.Get(), VertexBufferUpload.Get(), 0, 0, 1, &VertexData);

			Transition(MBDx12->VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}

		const uint32 IndexBufferSize = (InMeshData->GetIndicesCount() * (InMeshData->GetIndexType() == EIT_16BIT ? sizeof(uint16) : sizeof(uint32)));

		// Create the index buffer resource in the GPU's default heap and copy index data into it using the upload heap.
		// The upload resource must not be released until after the GPU has finished using it.
		ComPtr<ID3D12Resource> IndexBufferUpload;

		CD3DX12_RESOURCE_DESC IndexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize);
		VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&IndexBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&MBDx12->IndexBuffer)));

		VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&IndexBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&IndexBufferUpload)));

		DX_SETNAME(MBDx12->IndexBuffer.Get(), MeshBuffer->GetResourceName() + "-ib");

		// Upload the index buffer to the GPU.
		{
			D3D12_SUBRESOURCE_DATA IndexData = {};
			IndexData.pData = reinterpret_cast<const uint8*>(InMeshData->GetPSData());
			IndexData.RowPitch = IndexBufferSize;
			IndexData.SlicePitch = IndexData.RowPitch;

			UpdateSubresources(CommandList.Get(), MBDx12->IndexBuffer.Get(), IndexBufferUpload.Get(), 0, 0, 1, &IndexData);

			Transition(MBDx12->IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		}

		FlushResourceBarriers(CommandList.Get());

		// Create vertex/index buffer views.
		MBDx12->VertexBufferView.BufferLocation = MBDx12->VertexBuffer->GetGPUVirtualAddress();
		MBDx12->VertexBufferView.StrideInBytes = InMeshData->GetStride();
		MBDx12->VertexBufferView.SizeInBytes = BufferSize;

		MBDx12->IndexBufferView.BufferLocation = MBDx12->IndexBuffer->GetGPUVirtualAddress();
		MBDx12->IndexBufferView.SizeInBytes = IndexBufferSize;
		MBDx12->IndexBufferView.Format = InMeshData->GetIndexType() == EIT_16BIT ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

		// Hold resources used here
		HoldResourceReference(MeshBuffer);
		HoldResourceReference(VertexBufferUpload);
		HoldResourceReference(IndexBufferUpload);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Returns required size of a buffer to be used for data upload
	inline uint64 GetRequiredIntermediateSize(
		_In_ ID3D12Resource* pDestinationResource,
		_In_range_(0, D3D12_REQ_SUBRESOURCES) uint32 FirstSubresource,
		_In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) uint32 NumSubresources)
	{
		D3D12_RESOURCE_DESC Desc = pDestinationResource->GetDesc();
		uint64 RequiredSize = 0;

		ID3D12Device* pDevice;
		pDestinationResource->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
		pDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, 0, nullptr, nullptr, nullptr, &RequiredSize);
		pDevice->Release();

		return RequiredSize;
	}

	inline DXGI_FORMAT GetBaseFormat(DXGI_FORMAT defaultFormat)
	{
		switch (defaultFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_TYPELESS;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8A8_TYPELESS;

		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8X8_TYPELESS;

			// 32-bit Z w/ Stencil
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return DXGI_FORMAT_R32G8X24_TYPELESS;

			// No Stencil
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_R32_TYPELESS;

			// 24-bit Z
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return DXGI_FORMAT_R24G8_TYPELESS;

			// 16-bit Z w/o Stencil
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
			return DXGI_FORMAT_R16_TYPELESS;

		default:
			return defaultFormat;
		}
	}

	inline DXGI_FORMAT GetDSVFormat(DXGI_FORMAT defaultFormat)
	{
		switch (defaultFormat)
		{
			// 32-bit Z w/ Stencil
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

			// No Stencil
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_D32_FLOAT;

			// 24-bit Z
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;

			// 16-bit Z w/o Stencil
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
			return DXGI_FORMAT_D16_UNORM;

		default:
			return defaultFormat;
		}
	}

	inline DXGI_FORMAT GetDepthSrvFormat(DXGI_FORMAT defaultFormat)
	{
		switch (defaultFormat)
		{
			// 32-bit Z w/ Stencil
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

			// No Stencil
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;

			// 24-bit Z
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

			// 16-bit Z w/o Stencil
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
			return DXGI_FORMAT_R16_UNORM;

		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}
	bool FRHIDx12::UpdateHardwareResource(FTexturePtr Texture)
	{
		FTextureDx12 * TexDx12 = static_cast<FTextureDx12*>(Texture.get());
		if (TexDx12->TextureResource.IsInited())
			return true;
		const TTextureDesc& Desc = TexDx12->GetDesc();
		DXGI_FORMAT DxgiFormat = GetDxPixelFormat(Desc.Format);
		const bool IsCubeMap = Desc.Type == ETT_TEXTURE_CUBE;

		// do not have texture data, Create a empty texture (used for render target usually).
		D3D12_CLEAR_VALUE ClearValue = {};
		D3D12_RESOURCE_DESC TextureDx12Desc = {};
		TextureDx12Desc.Alignment = 0;
		TextureDx12Desc.DepthOrArraySize = 1;
		TextureDx12Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		TextureDx12Desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		if ((Desc.Flags & ETF_RT_COLORBUFFER) != 0)
		{
			TextureDx12Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			ClearValue.Color[0] = 0.0;
			ClearValue.Color[1] = 0.0;
			ClearValue.Color[2] = 0.0;
			ClearValue.Color[3] = 0.0;
		}
		if ((Desc.Flags & ETF_RT_DSBUFFER) != 0)
		{
			TextureDx12Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			ClearValue.DepthStencil.Depth = 1.f;
			ClearValue.DepthStencil.Stencil = 0;
		}
		TextureDx12Desc.Format = GetBaseFormat(DxgiFormat);
		TextureDx12Desc.Width = Desc.Width;
		TextureDx12Desc.Height = Desc.Height;
		TextureDx12Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		TextureDx12Desc.MipLevels = Desc.Mips;
		TextureDx12Desc.SampleDesc.Count = 1;
		TextureDx12Desc.SampleDesc.Quality = 0;

		ClearValue.Format = DxgiFormat;

		TexDx12->TextureResource.CreateResource(
			D3dDevice.Get(),
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&TextureDx12Desc,
			D3D12_RESOURCE_STATE_COMMON,
			&ClearValue
		);
		DX_SETNAME(TexDx12->TextureResource.GetResource(), Texture->GetResourceName());

		// Describe and create a SRV for the texture.
		//D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};

		//SRVDesc.Format = DxgiFormat;
		//if ((Desc.Flags & ETF_RT_DSBUFFER) != 0)
		//{
		//	SRVDesc.Format = GetDepthSrvFormat(DxgiFormat);
		//}
		//SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		//SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		//SRVDesc.Texture2D.MipLevels = Desc.Mips;
		//SRVDesc.Texture2D.MostDetailedMip = 0;

		//D3D12_CPU_DESCRIPTOR_HANDLE SrvDescriptor = GetCpuDescriptorHandle(Texture);
		//D3dDevice->CreateShaderResourceView(TexDx12->TextureResource.GetResource(), &SRVDesc, SrvDescriptor);

		HoldResourceReference(Texture);

		return true;
	}

	bool FRHIDx12::UpdateHardwareResource(FTexturePtr Texture, TTexturePtr InTexData)
	{
		TI_ASSERT(InTexData != nullptr);
		FTextureDx12 * TexDx12 = static_cast<FTextureDx12*>(Texture.get());
		const TTextureDesc& Desc = TexDx12->GetDesc();
		DXGI_FORMAT DxgiFormat = GetDxPixelFormat(Desc.Format);
		if (Desc.SRGB)
		{
			DxgiFormat = GetSRGBFormat(DxgiFormat);
		}
		const bool IsCubeMap = Desc.Type == ETT_TEXTURE_CUBE;

		// Create texture resource and fill with texture data.
#if defined (TIX_DEBUG)
		Texture->SetResourceName(InTexData->GetResourceName());
#endif

		// Note: ComPtr's are CPU objects but this resource needs to stay in scope until
		// the command list that references it has finished executing on the GPU.
		// We will flush the GPU at the end of this method to ensure the resource is not
		// prematurely destroyed.
		ComPtr<ID3D12Resource> TextureUploadHeap;
		const int32 ArraySize = IsCubeMap ? 6 : 1;

		TI_ASSERT(DxgiFormat != DXGI_FORMAT_UNKNOWN);
		// Describe and create a Texture2D.
		D3D12_RESOURCE_DESC TextureDx12Desc = {};
		TextureDx12Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		TextureDx12Desc.Alignment = 0;
		TextureDx12Desc.Width = Desc.Width;
		TextureDx12Desc.Height = Desc.Height;
		TextureDx12Desc.DepthOrArraySize = ArraySize;
		TextureDx12Desc.MipLevels = Desc.Mips;
		TextureDx12Desc.Format = DxgiFormat;
		TextureDx12Desc.SampleDesc.Count = 1;
		TextureDx12Desc.SampleDesc.Quality = 0;
		TextureDx12Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		TextureDx12Desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		TexDx12->TextureResource.CreateResource(
			D3dDevice.Get(),
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&TextureDx12Desc,
			D3D12_RESOURCE_STATE_COPY_DEST);

		const int32 SubResourceNum = ArraySize * Desc.Mips;
		const uint64 uploadBufferSize = GetRequiredIntermediateSize(TexDx12->TextureResource.GetResource(), 0, SubResourceNum);

		// Create the GPU upload buffer.
		VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&TextureUploadHeap)));

		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the Texture2D.

		D3D12_SUBRESOURCE_DATA* TextureDatas = ti_new D3D12_SUBRESOURCE_DATA[SubResourceNum];
		const TVector<TTexture::TSurface*>& TextureSurfaces = InTexData->GetSurfaces();
		TI_ASSERT(SubResourceNum == TextureSurfaces.size());
		for (int32 s = 0; s < SubResourceNum; ++s)
		{
			D3D12_SUBRESOURCE_DATA& texData = TextureDatas[s];
			const TTexture::TSurface* Surface = TextureSurfaces[s];
			texData.pData = Surface->Data;
			texData.RowPitch = Surface->RowPitch;
			texData.SlicePitch = Surface->DataSize;
		}

		UpdateSubresources(CommandList.Get(), TexDx12->TextureResource.GetResource(), TextureUploadHeap.Get(), 0, 0, SubResourceNum, TextureDatas);
		Transition(&TexDx12->TextureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		DX_SETNAME(TexDx12->TextureResource.GetResource(), Texture->GetResourceName());

		ti_delete[] TextureDatas;

		// Describe and create a SRV for the texture.
		//D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		//SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		//SRVDesc.Format = TextureDx12Desc.Format;
		//SRVDesc.ViewDimension = IsCubeMap ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
		//if (IsCubeMap)
		//{
		//	SRVDesc.TextureCube.MipLevels = TextureDx12Desc.MipLevels;
		//}
		//else
		//{
		//	SRVDesc.Texture2D.MipLevels = TextureDx12Desc.MipLevels;
		//}
		//D3D12_CPU_DESCRIPTOR_HANDLE Descriptor = GetCpuDescriptorHandle(Texture);
		//D3dDevice->CreateShaderResourceView(TexDx12->TextureResource.GetResource(), &SRVDesc, Descriptor);

		FlushResourceBarriers(CommandList.Get());
		// Hold resources used here
		HoldResourceReference(Texture);
		HoldResourceReference(TextureUploadHeap);

		return true;
	}

	inline FShaderDx12* GetDx12Shader(TPipelinePtr Pipeline, E_SHADER_STAGE Stage)
	{
		TI_ASSERT(0);
		if (Pipeline->GetDesc().Shader != nullptr)
		{
			//return static_cast<FShaderDx12*>(Pipeline->GetDesc().Shaders[Stage]->ShaderResource.get());
		}
		return nullptr;
	}

	bool FRHIDx12::UpdateHardwareResource(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc)
	{
#if defined (TIX_DEBUG)
		Pipeline->SetResourceName(InPipelineDesc->GetResourceName());
#endif
		FPipelineDx12 * PipelineDx12 = static_cast<FPipelineDx12*>(Pipeline.get());
		const TPipelineDesc& Desc = InPipelineDesc->GetDesc();

		TVector<E_MESH_STREAM_INDEX> Streams = TMeshBuffer::GetSteamsFromFormat(Desc.VsFormat);
		TVector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
		InputLayout.resize(Streams.size());

		// Fill layout desc
		uint32 VertexDataOffset = 0;
		for (uint32 i = 0; i < InputLayout.size(); ++i)
		{
			E_MESH_STREAM_INDEX Stream = Streams[i];
			D3D12_INPUT_ELEMENT_DESC& InputElement = InputLayout[i];
			InputElement.SemanticName = TMeshBuffer::SemanticName[Stream];
			InputElement.SemanticIndex = 0;
			InputElement.Format = k_MESHBUFFER_STREAM_FORMAT_MAP[Stream];
			InputElement.InputSlot = 0;
			InputElement.AlignedByteOffset = VertexDataOffset;
			VertexDataOffset += TMeshBuffer::SemanticSize[Stream];
			InputElement.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			InputElement.InstanceDataStepRate = 0;
		}

		FShaderBindingPtr Binding = Pipeline->GetShaderBinding();
		TI_ASSERT(Binding != nullptr);
		FRootSignatureDx12 * PipelineRS = static_cast<FRootSignatureDx12*>(Binding.get());

		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
		state.InputLayout = { &(InputLayout[0]), uint32(InputLayout.size()) };
		state.pRootSignature = PipelineRS->Get();
		TI_ASSERT(InPipelineDesc->GetDesc().Shader != nullptr);

		FShaderDx12 * ShaderDx12 = static_cast<FShaderDx12*>(InPipelineDesc->GetDesc().Shader->ShaderResource.get());

		state.VS = { ShaderDx12->ShaderCodes[ESS_VERTEX_SHADER].GetBuffer(), uint32(ShaderDx12->ShaderCodes[ESS_VERTEX_SHADER].GetLength()) };
		if (ShaderDx12->ShaderCodes[ESS_PIXEL_SHADER].GetLength() > 0)
		{
			state.PS = { ShaderDx12->ShaderCodes[ESS_PIXEL_SHADER].GetBuffer(), uint32(ShaderDx12->ShaderCodes[ESS_PIXEL_SHADER].GetLength()) };
		}
		if (ShaderDx12->ShaderCodes[ESS_DOMAIN_SHADER].GetLength() > 0)
		{
			state.PS = { ShaderDx12->ShaderCodes[ESS_DOMAIN_SHADER].GetBuffer(), uint32(ShaderDx12->ShaderCodes[ESS_DOMAIN_SHADER].GetLength()) };
		}
		if (ShaderDx12->ShaderCodes[ESS_HULL_SHADER].GetLength() > 0)
		{
			state.PS = { ShaderDx12->ShaderCodes[ESS_HULL_SHADER].GetBuffer(), uint32(ShaderDx12->ShaderCodes[ESS_HULL_SHADER].GetLength()) };
		}
		if (ShaderDx12->ShaderCodes[ESS_GEOMETRY_SHADER].GetLength() > 0)
		{
			state.PS = { ShaderDx12->ShaderCodes[ESS_GEOMETRY_SHADER].GetBuffer(), uint32(ShaderDx12->ShaderCodes[ESS_GEOMETRY_SHADER].GetLength()) };
		}

		MakeDx12RasterizerDesc(Desc, state.RasterizerState);
		MakeDx12BlendState(Desc, state.BlendState);
		MakeDx12DepthStencilState(Desc, state.DepthStencilState);
		state.SampleMask = UINT_MAX;
		state.PrimitiveTopologyType = k_PRIMITIVE_D3D12_TYPE_MAP[Desc.PrimitiveType];
		TI_ASSERT(D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED != state.PrimitiveTopologyType);
		state.NumRenderTargets = Desc.RTCount;
		TI_ASSERT(Desc.RTCount > 0);
		for (int32 r = 0 ; r < Desc.RTCount; ++r)
		{
			state.RTVFormats[r] = GetDxPixelFormat(Desc.RTFormats[r]);
		}
		if (Desc.DepthFormat != EPF_UNKNOWN)
		{
			state.DSVFormat = GetDxPixelFormat(Desc.DepthFormat);
		}
		else
		{
			state.DSVFormat = DXGI_FORMAT_UNKNOWN;
		}
		TI_ASSERT(DXGI_FORMAT_UNKNOWN != state.RTVFormats[0] || DXGI_FORMAT_UNKNOWN != state.DSVFormat);
		state.SampleDesc.Count = 1;

		VALIDATE_HRESULT(D3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&(PipelineDx12->PipelineState))));
		DX_SETNAME(PipelineDx12->PipelineState.Get(), Pipeline->GetResourceName());

		// Shader data can be deleted once the pipeline state is created.
		ShaderDx12->ReleaseShaderCode();
		HoldResourceReference(Pipeline);

		return true;
	}

	static const int32 UniformBufferAlignSize = 256;
	bool FRHIDx12::UpdateHardwareResource(FUniformBufferPtr UniformBuffer, void* InData)
	{
		FUniformBufferDx12 * UniformBufferDx12 = static_cast<FUniformBufferDx12*>(UniformBuffer.get());

		const int32 AlignedDataSize = ti_align(UniformBuffer->GetStructSize(), UniformBufferAlignSize);
		CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(AlignedDataSize);

		CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&constantBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&UniformBufferDx12->ConstantBuffer)));

		//D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		//cbvDesc.BufferLocation = UniformBufferDx12->ConstantBuffer->GetGPUVirtualAddress();
		//cbvDesc.SizeInBytes = AlignedDataSize;
		//D3D12_CPU_DESCRIPTOR_HANDLE Descriptor = GetCpuDescriptorHandle(UniformBuffer);
		//D3dDevice->CreateConstantBufferView(&cbvDesc, Descriptor);

		// Map the constant buffers.
		uint8 * MappedConstantBuffer = nullptr;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		VALIDATE_HRESULT(UniformBufferDx12->ConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&MappedConstantBuffer)));
		memcpy(MappedConstantBuffer, InData, UniformBuffer->GetStructSize());
		if (AlignedDataSize - UniformBuffer->GetStructSize() > 0)
		{
			memset(MappedConstantBuffer + UniformBuffer->GetStructSize(), 0, AlignedDataSize - UniformBuffer->GetStructSize());
		}
		UniformBufferDx12->ConstantBuffer->Unmap(0, nullptr);
		HoldResourceReference(UniformBuffer);

		return true;
	}

	bool FRHIDx12::UpdateHardwareResource(FRenderTargetPtr RenderTarget)
	{
		FRenderTargetDx12 * RTDx12 = static_cast<FRenderTargetDx12*>(RenderTarget.get());
		// Create render target render resource tables
		int32 ColorBufferCount = RenderTarget->GetColorBufferCount();
		TI_ASSERT(RTDx12->RTColorTable == nullptr);
		RTDx12->RTColorTable = RenderResourceHeap[EHT_RENDERTARGET].AllocateTable(ColorBufferCount);
		for (int32 i = 0; i < ColorBufferCount; ++i)
		{
			const FRenderTarget::RTBuffer& ColorBuffer = RenderTarget->GetColorBuffer(i);
			RTDx12->RTColorTable->PutRTColorInTable(ColorBuffer.Texture, i);
		}

		// Depth stencil buffers
		{
			const FRenderTarget::RTBuffer& DepthStencilBuffer = RenderTarget->GetDepthStencilBuffer();
			FTexturePtr DSBufferTexture = DepthStencilBuffer.Texture;
			if (DSBufferTexture != nullptr)
			{
				TI_ASSERT(RTDx12->RTDepthTable == nullptr);
				RTDx12->RTDepthTable = RenderResourceHeap[EHT_DEPTHSTENCIL].AllocateTable(1);
				RTDx12->RTDepthTable->PutRTDepthInTable(DSBufferTexture, 0);
			}
		}
		return true;
	}

	bool FRHIDx12::UpdateHardwareResource(FShaderBindingPtr ShaderBindingResource, const TVector<TBindingParamInfo>& BindingInfos)
	{
		FRootSignatureDx12 * RootSignatureDx12 = static_cast<FRootSignatureDx12*>(ShaderBindingResource.get());
		TI_ASSERT(RootSignatureDx12->ParamArray.size() == BindingInfos.size());
		TI_ASSERT((int32)RootSignatureDx12->SamplerArray.size() == FRHIConfig::StaticSamplerNum);

		const int32 NumBindings = (int32)RootSignatureDx12->ParamArray.size();
		const int32 NumSamplers = (int32)RootSignatureDx12->SamplerArray.size();
		// Init static sampler
		RootSignatureDx12->InitStaticSampler(0, DefaultSampler, D3D12_SHADER_VISIBILITY_PIXEL);

		// Init shader params
		for (int32 i = 0; i < NumBindings; ++i)
		{
			const TBindingParamInfo& Info = BindingInfos[i];
			D3D12_SHADER_VISIBILITY ShaderVisibility = GetDxShaderVisibilityFromTiX((E_SHADER_STAGE)Info.BindingStage);
			switch (Info.BindingType)
			{
			case BINDING_LIGHTS:
				RootSignatureDx12->GetParameter(i).InitAsConstantBuffer(Info.BindingRegister, ShaderVisibility);
				break;
			case BINDING_UNIFORMBUFFER:
				RootSignatureDx12->GetParameter(i).InitAsConstantBuffer(Info.BindingRegister, ShaderVisibility);
				break;
			case BINDING_UNIFORMBUFFER_TABLE:
				RootSignatureDx12->GetParameter(i).InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, Info.BindingRegister, Info.BindingSize, ShaderVisibility);
				break;
			case BINDING_TEXTURE_TABLE:
				RootSignatureDx12->GetParameter(i).InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Info.BindingRegister, Info.BindingSize, ShaderVisibility);
				break;
			default:
				TI_ASSERT(0);
				break;
			}
		}
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		RootSignatureDx12->Finalize(D3dDevice.Get(), rootSignatureFlags);

		HoldResourceReference(ShaderBindingResource);

		return true;
	}

	bool FRHIDx12::UpdateHardwareResource(FShaderPtr ShaderResource)
	{
		// Dx12 shader only need load byte code.
		FShaderDx12 * ShaderDx12 = static_cast<FShaderDx12*>(ShaderResource.get());

		for (int32 s = 0; s < ESS_COUNT; ++s)
		{
			TString ShaderName = ShaderDx12->GetShaderName((E_SHADER_STAGE)s);
			if (!ShaderName.empty())
			{
				if (ShaderName.rfind(".cso") == TString::npos)
					ShaderName += ".cso";

				// Load shader code
				TFile File;
				if (File.Open(ShaderName, EFA_READ))
				{
					ShaderDx12->ShaderCodes[s].Reset();
					ShaderDx12->ShaderCodes[s].Put(File);
					File.Close();
				}
				else
				{
					_LOG(Fatal, "Failed to load shader code [%s].\n", ShaderName.c_str());
				}
			}
		}

		return true;
	}

	//bool FRHIDx12::UpdateHardwareResource(FRenderTargetPtr RenderTarget)
	//{
	//	FRenderTargetDx12 * RenderTargetDx12 = static_cast<FRenderTargetDx12*>(RenderTarget.get());

	//	TI_TODO("First here.!!!!!!!");
	//	TI_ASSERT(0);
	//	// Create Render target view
	//	// Color buffers
	//	int32 ColorBufferCount = 0;
	//	for (int32 i = 0; i < ERTC_COUNT; ++i)
	//	{
	//		const FRenderTarget::RTBuffer& ColorBuffer = RenderTarget->GetColorBuffer(i);
	//		if (ColorBuffer.BufferIndex != ERTC_INVALID)
	//		{
	//			//FTexturePtr ColorBufferTexture = ColorBuffer.Texture;
	//			//TI_ASSERT(ColorBufferTexture != nullptr);
	//			//FTextureDx12 * TexDx12 = static_cast<FTextureDx12*>(ColorBufferTexture.get());
	//			//TI_ASSERT(TexDx12->TextureResource.GetResource() != nullptr);

	//			//D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
	//			//RTVDesc.Format = k_PIXEL_FORMAT_MAP[ColorBufferTexture->GetDesc().Format];
	//			//TI_ASSERT(RTVDesc.Format != DXGI_FORMAT_UNKNOWN);
	//			//RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	//			//RTVDesc.Texture2D.MipSlice = 0;
	//			//RTVDesc.Texture2D.PlaneSlice = 0;

	//			//TI_ASSERT(RenderTargetDx12->RTColorDescriptor[i].ptr == 0);
	//			//ColorBuffer.RTResource->InitRenderResourceHeapSlot();
	//			//RenderTargetDx12->RTColorDescriptor[i] = GetCpuDescriptorHandle(ColorBuffer.RTResource);
	//			//D3dDevice->CreateRenderTargetView(TexDx12->TextureResource.GetResource(), &RTVDesc, RenderTargetDx12->RTColorDescriptor[i]);

	//			++ColorBufferCount;
	//		}
	//	}
	//	TI_ASSERT(RenderTarget->GetColorBufferCount() == ColorBufferCount);

	//	// Depth stencil buffers
	//	{
	//		const FRenderTarget::RTBuffer& DepthStencilBuffer = RenderTarget->GetDepthStencilBuffer();
	//		FTexturePtr DSBufferTexture = DepthStencilBuffer.Texture;
	//		if (DSBufferTexture != nullptr)
	//		{
	//			//FTextureDx12 * TexDx12 = static_cast<FTextureDx12*>(DSBufferTexture.get());
	//			//TI_ASSERT(TexDx12->TextureResource.GetResource() != nullptr);

	//			//DXGI_FORMAT DxgiFormat = k_PIXEL_FORMAT_MAP[DSBufferTexture->GetDesc().Format];
	//			//TI_ASSERT(DXGI_FORMAT_UNKNOWN != DxgiFormat);

	//			//D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc;
	//			//DsvDesc.Format = GetDSVFormat(DxgiFormat);
	//			//DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//			//DsvDesc.Texture2D.MipSlice = 0;
	//			//DsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	//			//TI_ASSERT(RenderTargetDx12->RTDSDescriptor.ptr == 0);
	//			//DepthStencilBuffer.RTResource->InitRenderResourceHeapSlot();
	//			//RenderTargetDx12->RTDSDescriptor = GetCpuDescriptorHandle(DepthStencilBuffer.RTResource);
	//			//D3dDevice->CreateDepthStencilView(TexDx12->TextureResource.GetResource(), &DsvDesc, RenderTargetDx12->RTDSDescriptor);
	//		}
	//	}
	//	return true;
	//}

	void FRHIDx12::PutUniformBufferInHeap(FUniformBufferPtr InUniformBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot)
	{
		FUniformBufferDx12 * UniformBufferDx12 = static_cast<FUniformBufferDx12*>(InUniformBuffer.get());

		const int32 AlignedDataSize = ti_align(InUniformBuffer->GetStructSize(), UniformBufferAlignSize);
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = UniformBufferDx12->ConstantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = AlignedDataSize;
		D3D12_CPU_DESCRIPTOR_HANDLE Descriptor = GetCpuDescriptorHandle(InHeapType, InHeapSlot);
		D3dDevice->CreateConstantBufferView(&cbvDesc, Descriptor);
	}

	void FRHIDx12::PutTextureInHeap(FTexturePtr InTexture, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot)
	{
		FTextureDx12 * TexDx12 = static_cast<FTextureDx12*>(InTexture.get());

		const TTextureDesc& Desc = InTexture->GetDesc();
		DXGI_FORMAT DxgiFormat = GetDxPixelFormat(Desc.Format);
		const bool IsCubeMap = Desc.Type == ETT_TEXTURE_CUBE;
		TI_ASSERT(DxgiFormat != DXGI_FORMAT_UNKNOWN);

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		DXGI_FORMAT DepthSrvFormat = GetDepthSrvFormat(DxgiFormat);
		if (DepthSrvFormat != DXGI_FORMAT_UNKNOWN)
		{
			// A depth texture
			SRVDesc.Format = DepthSrvFormat;
		}
		else
		{
			TI_TODO("Refactor SRGB to Engine Level.");
			if (Desc.SRGB)
			{
				DxgiFormat = GetSRGBFormat(DxgiFormat);
			}
			// This is not a depth related buffer, treat as a normal texture.
			SRVDesc.Format = DxgiFormat;
		}
		SRVDesc.ViewDimension = IsCubeMap ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
		if (IsCubeMap)
		{
			SRVDesc.TextureCube.MipLevels = Desc.Mips;
		}
		else
		{
			SRVDesc.Texture2D.MipLevels = Desc.Mips;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE Descriptor = GetCpuDescriptorHandle(InHeapType, InHeapSlot);
		D3dDevice->CreateShaderResourceView(TexDx12->TextureResource.GetResource(), &SRVDesc, Descriptor);
	}

	void FRHIDx12::PutRTColorInHeap(FTexturePtr InTexture, uint32 InHeapSlot)
	{
		FTextureDx12 * TexDx12 = static_cast<FTextureDx12*>(InTexture.get());
		TI_ASSERT(TexDx12->TextureResource.GetResource() != nullptr);

		D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
		RTVDesc.Format = GetDxPixelFormat(InTexture->GetDesc().Format);
		TI_ASSERT(RTVDesc.Format != DXGI_FORMAT_UNKNOWN);
		RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RTVDesc.Texture2D.MipSlice = 0;
		RTVDesc.Texture2D.PlaneSlice = 0;

		D3D12_CPU_DESCRIPTOR_HANDLE Descriptor = GetCpuDescriptorHandle(EHT_RENDERTARGET, InHeapSlot);
		D3dDevice->CreateRenderTargetView(TexDx12->TextureResource.GetResource(), &RTVDesc, Descriptor);
	}

	void FRHIDx12::PutRTDepthInHeap(FTexturePtr InTexture, uint32 InHeapSlot)
	{
		FTextureDx12 * TexDx12 = static_cast<FTextureDx12*>(InTexture.get());
		TI_ASSERT(TexDx12->TextureResource.GetResource() != nullptr);

		DXGI_FORMAT DxgiFormat = GetDxPixelFormat(InTexture->GetDesc().Format);
		TI_ASSERT(DXGI_FORMAT_UNKNOWN != DxgiFormat);

		D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc;
		DsvDesc.Format = GetDSVFormat(DxgiFormat);
		DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		DsvDesc.Texture2D.MipSlice = 0;
		DsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CPU_DESCRIPTOR_HANDLE Descriptor = GetCpuDescriptorHandle(EHT_DEPTHSTENCIL, InHeapSlot);
		D3dDevice->CreateDepthStencilView(TexDx12->TextureResource.GetResource(), &DsvDesc, Descriptor);
	}

	void FRHIDx12::SetPipeline(FPipelinePtr InPipeline)
	{
		if (CurrentBoundResource.Pipeline != InPipeline)
		{
			FPipelineDx12* PipelineDx12 = static_cast<FPipelineDx12*>(InPipeline.get());

			FShaderBindingPtr ShaderBinding = InPipeline->GetShaderBinding();
			TI_ASSERT(ShaderBinding != nullptr);
			if (CurrentBoundResource.ShaderBinding != ShaderBinding)
			{
				FRootSignatureDx12 * RSDx12 = static_cast<FRootSignatureDx12*>(ShaderBinding.get());
				CommandList->SetGraphicsRootSignature(RSDx12->Get());
				CurrentBoundResource.ShaderBinding = ShaderBinding;
			}

			CommandList->SetPipelineState(PipelineDx12->PipelineState.Get());

			HoldResourceReference(InPipeline);

			CurrentBoundResource.Pipeline = InPipeline;
		}
	}

	void FRHIDx12::SetMeshBuffer(FMeshBufferPtr InMeshBuffer)
	{
		FMeshBufferDx12* MBDx12 = static_cast<FMeshBufferDx12*>(InMeshBuffer.get());

		CommandList->IASetPrimitiveTopology(k_PRIMITIVE_TYPE_MAP[InMeshBuffer->GetPrimitiveType()]);
		CommandList->IASetVertexBuffers(0, 1, &MBDx12->VertexBufferView);
		CommandList->IASetIndexBuffer(&MBDx12->IndexBufferView);

		HoldResourceReference(InMeshBuffer);
	}

	void FRHIDx12::SetUniformBuffer(int32 BindIndex, FUniformBufferPtr InUniformBuffer)
	{
		FUniformBufferDx12* UBDx12 = static_cast<FUniformBufferDx12*>(InUniformBuffer.get());

		// Bind the current frame's constant buffer to the pipeline.
		CommandList->SetGraphicsRootConstantBufferView(BindIndex, UBDx12->ConstantBuffer->GetGPUVirtualAddress());

		HoldResourceReference(InUniformBuffer);
	}

	void FRHIDx12::SetRenderResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable)
	{
		// Bind the current frame's constant buffer to the pipeline.
		D3D12_GPU_DESCRIPTOR_HANDLE Descriptor = GetGpuDescriptorHandle(RenderResourceTable->GetHeapType(), RenderResourceTable->GetStartIndex());
		CommandList->SetGraphicsRootDescriptorTable(BindIndex, Descriptor);

		HoldResourceReference(RenderResourceTable);
	}

	void FRHIDx12::SetShaderTexture(int32 BindIndex, FTexturePtr InTexture)
	{
		FTextureDx12* TexDx12 = static_cast<FTextureDx12*>(InTexture.get());
		//The GPU virtual address of the Buffer.Textures are not supported.D3D12_GPU_VIRTUAL_ADDRESS is a typedef'd alias of UINT64.
		// Bind texture to pipeline
		//CommandList->SetGraphicsRootShaderResourceView(BindIndex, TexDx12->TextureResource.GetResource()->GetGPUVirtualAddress());
		TI_ASSERT(0);

		HoldResourceReference(InTexture);
	}

	void FRHIDx12::SetStencilRef(uint32 InRefValue)
	{
		CommandList->OMSetStencilRef(InRefValue);
	}

	void FRHIDx12::DrawPrimitiveIndexedInstanced(
		uint32 IndexCountPerInstance,
		uint32 InstanceCount,
		uint32 StartIndexLocation,
		int32 BaseVertexLocation,
		uint32 StartInstanceLocation)
	{
		CommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
	}

	void FRHIDx12::SetViewport(const FViewport& VP)
	{
		FRHI::SetViewport(VP);

		D3D12_VIEWPORT ViewportDx = { float(VP.Left), float(VP.Top), float(VP.Width), float(VP.Height), 0.f, 1.f };
		CommandList->RSSetViewports(1, &ViewportDx);
	}

	void FRHIDx12::SetRenderTarget(FRenderTargetPtr RT)
	{
		// Transition Color buffer to D3D12_RESOURCE_STATE_RENDER_TARGET
		FRenderTargetDx12 * RTDx12 = static_cast<FRenderTargetDx12*>(RT.get());
		const int32 CBCount = RT->GetColorBufferCount();
		for (int32 cb = 0; cb < CBCount; ++cb)
		{
			FTexturePtr Texture = RT->GetColorBuffer(cb).Texture;
			TI_ASSERT(Texture != nullptr);	// Color can be NULL ?
			FTextureDx12* TexDx12 = static_cast<FTextureDx12*>(Texture.get());
			Transition(&TexDx12->TextureResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		// Transition Depth buffer to D3D12_RESOURCE_STATE_DEPTH_WRITE
		{
			FTexturePtr Texture = RT->GetDepthStencilBuffer().Texture;
			if (Texture != nullptr)
			{
				FTextureDx12* TexDx12 = static_cast<FTextureDx12*>(Texture.get());
				Transition(&TexDx12->TextureResource, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			}
		}
		FlushResourceBarriers(CommandList.Get());

		TVector<D3D12_CPU_DESCRIPTOR_HANDLE> RTVDescriptors;
		const D3D12_CPU_DESCRIPTOR_HANDLE* Rtv = nullptr;
		if (CBCount > 0)
		{
			FRenderResourceTablePtr ColorTable = RTDx12->RTColorTable;
			RTVDescriptors.reserve(CBCount); 
			for (int32 cb = 0 ; cb < CBCount ; ++ cb)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE Descriptor = GetCpuDescriptorHandle(EHT_RENDERTARGET, ColorTable->GetIndexAt(cb));
				RTVDescriptors.push_back(Descriptor);
			}
			Rtv = RTVDescriptors.data();
		}

		const D3D12_CPU_DESCRIPTOR_HANDLE* Dsv = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthDescriptor;
		FRenderResourceTablePtr DepthTable = RTDx12->RTDepthTable;
		if (DepthTable != nullptr && DepthTable->GetTableSize() != 0)
		{
			DepthDescriptor = GetCpuDescriptorHandle(EHT_DEPTHSTENCIL, DepthTable->GetIndexAt(0));
			Dsv = &DepthDescriptor;
		}

		// Set render target
		CommandList->OMSetRenderTargets(RT->GetColorBufferCount(), Rtv, false, Dsv);

		// Clear render target
		if (CBCount > 0)
		{
			for (int32 cb = 0; cb < CBCount; ++cb)
			{
				CommandList->ClearRenderTargetView(RTVDescriptors[cb], DirectX::Colors::Transparent, 0, nullptr);
			}
		}
		if (Dsv != nullptr)
		{
			CommandList->ClearDepthStencilView(*Dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		}
	}

	void FRHIDx12::PushRenderTarget(FRenderTargetPtr RT)
	{
		FRHI::PushRenderTarget(RT);

		SetRenderTarget(RT);
	}

	FRenderTargetPtr FRHIDx12::PopRenderTarget()
	{
		// Transition Color buffer to D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
		TI_ASSERT(RenderTargets.size() > 0);
		FRenderTargetPtr CurrentRT = RenderTargets.back();
		const int32 CBCount = CurrentRT->GetColorBufferCount();
		for (int32 cb = 0; cb < CBCount; ++cb)
		{
			FTexturePtr Texture = CurrentRT->GetColorBuffer(cb).Texture;
			TI_ASSERT(Texture != nullptr);
			FTextureDx12* TexDx12 = static_cast<FTextureDx12*>(Texture.get());
			Transition(&TexDx12->TextureResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}

		// Transition Depth buffer to D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
		{
			FTexturePtr Texture = CurrentRT->GetDepthStencilBuffer().Texture;
			if (Texture != nullptr)
			{
				FTextureDx12* TexDx12 = static_cast<FTextureDx12*>(Texture.get());
				Transition(&TexDx12->TextureResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			}
		}
		FlushResourceBarriers(CommandList.Get());

		// Pop rt
		FRenderTargetPtr RT = FRHI::PopRenderTarget();

		if (RT == nullptr)
		{
			// Set back to frame buffer
			D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = BackBufferDescriptors[CurrentFrame];
			D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilDescriptor;
			CommandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);
		}
		else
		{
			SetRenderTarget(RT);
		}
		return RT;
	}

	void FRHIDx12::InitRHIRenderResourceHeap(E_RENDER_RESOURCE_HEAP_TYPE Heap, uint32 HeapSize, uint32 HeapOffset)
	{
		RenderResourceHeap[Heap].Create(Heap, HeapSize, HeapOffset);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE FRHIDx12::GetCpuDescriptorHandle(E_RENDER_RESOURCE_HEAP_TYPE Heap, uint32 SlotIndex)
	{
		D3D12_DESCRIPTOR_HEAP_TYPE Dx12Heap = GetDxHeapTypeFromTiXHeap(Heap);
		return DescriptorHeaps[Dx12Heap].GetCpuDescriptorHandle(SlotIndex);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE FRHIDx12::GetGpuDescriptorHandle(E_RENDER_RESOURCE_HEAP_TYPE Heap, uint32 SlotIndex)
	{
		D3D12_DESCRIPTOR_HEAP_TYPE Dx12Heap = GetDxHeapTypeFromTiXHeap(Heap);
		return DescriptorHeaps[Dx12Heap].GetGpuDescriptorHandle(SlotIndex);
	}
}
#endif	// COMPILE_WITH_RHI_DX12