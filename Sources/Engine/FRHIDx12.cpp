/*
	TiX Engine v2.0 Copyright (C) 2018
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

// link libraries
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")

namespace tix
{
	FRHIDx12::FRHIDx12()
		: FRHI(ERHI_DX12)
		, CurrentFrame(0)
		, NumBarriersToFlush(0)
		, SrvDescriptorSize(0)
	{
		Init();

		// Create frame resource holders
		for (int32 i = 0; i < FRHIConfig::FrameBufferNum; ++i)
		{
			FrameResources[i] = ti_new FFrameResourcesDx12;
		}
	}

	FRHIDx12::~FRHIDx12()
	{
		// delete frame resource holders
		for (int32 i = 0; i < FRHIConfig::FrameBufferNum; ++i)
		{
			ti_delete FrameResources[i];
			FrameResources[i] = nullptr;
		}
	}

	void FRHIDx12::Init()
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

#if defined(_DEBUG)
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

		hr = D3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue));
		VALIDATE_HRESULT(hr);
		CommandQueue->SetName(L"CommandQueue");

		// Create descriptor heaps for render target views and depth stencil views.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FRHIConfig::FrameBufferNum;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = D3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&RtvHeap));
		VALIDATE_HRESULT(hr);
		RtvHeap->SetName(L"RtvHeap");

		RtvDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = D3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&DsvHeap));
		VALIDATE_HRESULT(hr);
		DsvHeap->SetName(L"DsvHeap");

		// Create command allocator and command list.
		for (uint32 n = 0; n < FRHIConfig::FrameBufferNum; n++)
		{
			VALIDATE_HRESULT(D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocators[n])));
			VALIDATE_HRESULT(D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocators[n].Get(), nullptr, IID_PPV_ARGS(&CommandLists[n])));
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
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		VALIDATE_HRESULT(D3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&SrvHeap)));

		SrvHeapHandle = SrvHeap->GetCPUDescriptorHandleForHeapStart();
		SrvDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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

		const DXGI_FORMAT BackBufferFormat = k_PIXEL_FORMAT_MAP[FRHIConfig::DefaultBackBufferFormat];
		const DXGI_FORMAT DepthBufferFormat = k_PIXEL_FORMAT_MAP[FRHIConfig::DefaultDepthBufferFormat];
		TI_ASSERT(BackBufferFormat != DXGI_FORMAT_UNKNOWN && DepthBufferFormat != DXGI_FORMAT_UNKNOWN);

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
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(RtvHeap->GetCPUDescriptorHandleForHeapStart());
			for (uint32 n = 0; n < FRHIConfig::FrameBufferNum; n++)
			{
				hr = SwapChain->GetBuffer(n, IID_PPV_ARGS(&BackBufferRTs[n]));
				VALIDATE_HRESULT(hr);
				D3dDevice->CreateRenderTargetView(BackBufferRTs[n].Get(), nullptr, rtvDescriptor);

				rtvDescriptor.Offset(RtvDescriptorSize);

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

			D3dDevice->CreateDepthStencilView(DepthStencil.Get(), &dsvDesc, DsvHeap->GetCPUDescriptorHandleForHeapStart());
		}

		// Create a root signature with a single constant buffer slot.
		{
			CD3DX12_DESCRIPTOR_RANGE range;
			CD3DX12_ROOT_PARAMETER parameter;

			range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
			parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

			CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
			descRootSignature.Init(1, &parameter, 0, nullptr, rootSignatureFlags);

			ComPtr<ID3DBlob> pSignature;
			ComPtr<ID3DBlob> pError;
			VALIDATE_HRESULT(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));
			VALIDATE_HRESULT(D3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
			RootSignature->SetName(L"RootSignature");
		}

		// Set the 3D rendering viewport to target the entire window.
		//m_screenViewport = { 0.0f, 0.0f, m_d3dRenderTargetSize.Width, m_d3dRenderTargetSize.Height, 0.0f, 1.0f };

	}

	void FRHIDx12::BeginFrame()
	{
		HRESULT hr;
		hr = CommandAllocators[CurrentFrame]->Reset();
		VALIDATE_HRESULT(hr);

		TI_ASSERT(NumBarriersToFlush == 0);
	}

	void FRHIDx12::EndFrame()
	{
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
	}

	FTexturePtr FRHIDx12::CreateTexture()
	{
		return ti_new FTextureDx12();
	}

	FTexturePtr FRHIDx12::CreateTexture(E_PIXEL_FORMAT Format, int32 Width, int32 Height)
	{
		return ti_new FTextureDx12(Format, Width, Height);
	}

	FMeshBufferPtr FRHIDx12::CreateMeshBuffer()
	{
		return ti_new FMeshBufferDx12();
	}

	FPipelinePtr FRHIDx12::CreatePipeline()
	{
		return ti_new FPipelineDx12();
	}

	// Prepare to render the next frame.
	void FRHIDx12::MoveToNextFrame()
	{
		HRESULT hr;
		// Schedule a Signal command in the queue.
		const uint64 currentFenceValue = FenceValues[CurrentFrame];
		hr = CommandQueue->Signal(Fence.Get(), currentFenceValue);
		VALIDATE_HRESULT(hr);

		// Advance the frame index.
		CurrentFrame = SwapChain->GetCurrentBackBufferIndex();

		// Check to see if the next frame is ready to start.
		if (Fence->GetCompletedValue() < FenceValues[CurrentFrame])
		{
			hr = Fence->SetEventOnCompletion(FenceValues[CurrentFrame], FenceEvent);
			WaitForSingleObjectEx(FenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		FenceValues[CurrentFrame] = currentFenceValue + 1;

		// Release resources references
		TI_TODO("Validat if CurrentFrame is the correct point to release.");
		FrameResources[CurrentFrame]->RemoveAllReferences();
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

	bool FRHIDx12::UpdateHardwareBuffer(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData)
	{
		TI_ASSERT(MeshBuffer->GetResourceFamily() == ERF_Dx12);
		FMeshBufferDx12 * MBDx12 = static_cast<FMeshBufferDx12*>(MeshBuffer.get());
		MeshBuffer->SetFromTMeshBuffer(InMeshData);

		ComPtr<ID3D12GraphicsCommandList> CommandList = CommandLists[CurrentFrame];
		FFrameResourcesDx12 * FrameResource = static_cast<FFrameResourcesDx12*>(FrameResources[CurrentFrame]);

		// Create the vertex buffer resource in the GPU's default heap and copy vertex data into it using the upload heap.
		// The upload resource must not be released until after the GPU has finished using it.
		ComPtr<ID3D12Resource> VertexBufferUpload;

		int32 BufferSize = InMeshData->GetVerticesCount() * InMeshData->GetStride();
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

		MBDx12->VertexBuffer->SetName(MeshBuffer->GetResourceWName().c_str());

		// Upload the vertex buffer to the GPU.
		{
			D3D12_SUBRESOURCE_DATA VertexData = {};
			VertexData.pData = reinterpret_cast<const uint8*>(InMeshData->GetVSData());
			VertexData.RowPitch = BufferSize;
			VertexData.SlicePitch = VertexData.RowPitch;

			UpdateSubresources(CommandList.Get(), MBDx12->VertexBuffer.Get(), VertexBufferUpload.Get(), 0, 0, 1, &VertexData);

			Transition(MBDx12->VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}

		const uint32 IndexBufferSize = sizeof(InMeshData->GetIndicesCount() * (InMeshData->GetIndexType() == EIT_16BIT ? sizeof(uint16) : sizeof(uint32)));

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

		MBDx12->IndexBuffer->SetName(MeshBuffer->GetResourceWName().c_str());

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

		// Hold resources used here
		FrameResource->HoldReference(MeshBuffer);
		FrameResource->HoldDxReference(VertexBufferUpload);
		FrameResource->HoldDxReference(IndexBufferUpload);

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

	void FRHIDx12::RecallDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE Handle)
	{
		AvaibleSrvHeapHandles.push_back(Handle);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE FRHIDx12::AllocateDescriptorHandle()
	{
		if (AvaibleSrvHeapHandles.size() > 0)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE ret = AvaibleSrvHeapHandles.back();
			AvaibleSrvHeapHandles.pop_back();
			return ret;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE ret = SrvHeapHandle;
		SrvHeapHandle.ptr += SrvDescriptorSize;
		return ret;
	}

	bool FRHIDx12::UpdateHardwareBuffer(FTexturePtr Texture, TTexturePtr InTexData)
	{
		TI_ASSERT(Texture->GetResourceFamily() == ERF_Dx12);
		FTextureDx12 * TexDx12 = static_cast<FTextureDx12*>(Texture.get());
		Texture->InitTextureInfo(InTexData);
		const TTextureDesc& Desc = InTexData->Desc;

		ComPtr<ID3D12GraphicsCommandList> CommandList = CommandLists[CurrentFrame];
		FFrameResourcesDx12 * FrameResource = static_cast<FFrameResourcesDx12*>(FrameResources[CurrentFrame]);

		// Note: ComPtr's are CPU objects but this resource needs to stay in scope until
		// the command list that references it has finished executing on the GPU.
		// We will flush the GPU at the end of this method to ensure the resource is not
		// prematurely destroyed.
		ComPtr<ID3D12Resource> TextureUploadHeap;

		DXGI_FORMAT DxgiFormat = k_PIXEL_FORMAT_MAP[Desc.Format];
		TI_ASSERT(DxgiFormat != DXGI_FORMAT_UNKNOWN);
		// Describe and create a Texture2D.
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = Desc.Mips;
		textureDesc.Format = DxgiFormat;
		textureDesc.Width = Texture->GetWidth();
		textureDesc.Height = Texture->GetHeight();
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&TexDx12->TextureResource)));

		const uint64 uploadBufferSize = GetRequiredIntermediateSize(TexDx12->TextureResource.Get(), 0, Desc.Mips);

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
		D3D12_SUBRESOURCE_DATA* TextureDatas = ti_new D3D12_SUBRESOURCE_DATA[Desc.Mips];
		const TVector<TTexture::TSurface*>& TextureSurfaces = InTexData->GetSurfaces();
		for (uint32 m = 0; m < Desc.Mips; ++m)
		{
			D3D12_SUBRESOURCE_DATA& texData = TextureDatas[m];
			const TTexture::TSurface* Surface = TextureSurfaces[m];
			texData.pData = Surface->Data;
			texData.RowPitch = Surface->RowPitch;
			texData.SlicePitch = Surface->DataSize;
		}

		UpdateSubresources(CommandList.Get(), TexDx12->TextureResource.Get(), TextureUploadHeap.Get(), 0, 0, Desc.Mips, TextureDatas);
		Transition(TexDx12->TextureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		ti_delete[] TextureDatas;

		// Describe and create a SRV for the texture.
		TexDx12->TexDescriptor = AllocateDescriptorHandle();
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		D3dDevice->CreateShaderResourceView(TexDx12->TextureResource.Get(), &srvDesc, TexDx12->TexDescriptor);

		FlushResourceBarriers(CommandList.Get());
		// Hold resources used here
		FrameResource->HoldReference(Texture);
		FrameResource->HoldDxReference(TextureUploadHeap);

		return true;
	}

	bool FRHIDx12::UpdateHardwareBuffer(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc)
	{
		TI_ASSERT(Pipeline->GetResourceFamily() == ERF_Dx12);
		FPipelineDx12 * PipelineDx12 = static_cast<FPipelineDx12*>(Pipeline.get());
		const TPipelineDesc& Desc = InPipelineDesc->Desc;

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

		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
		state.InputLayout = { &(InputLayout[0]), uint32(InputLayout.size()) };
		state.pRootSignature = RootSignature.Get();
		TI_ASSERT(InPipelineDesc->ShaderCode[ESS_VERTEX_SHADER].GetLength() > 0);
		state.VS = { InPipelineDesc->ShaderCode[ESS_VERTEX_SHADER].GetBuffer(), uint32(InPipelineDesc->ShaderCode[ESS_VERTEX_SHADER].GetLength()) };
		if (InPipelineDesc->ShaderCode[ESS_PIXEL_SHADER].GetLength() > 0)
		{
			state.PS = { InPipelineDesc->ShaderCode[ESS_PIXEL_SHADER].GetBuffer(), uint32(InPipelineDesc->ShaderCode[ESS_PIXEL_SHADER].GetLength()) };
		}
		if (InPipelineDesc->ShaderCode[ESS_DOMAIN_SHADER].GetLength() > 0)
		{
			state.DS = { InPipelineDesc->ShaderCode[ESS_DOMAIN_SHADER].GetBuffer(), uint32(InPipelineDesc->ShaderCode[ESS_DOMAIN_SHADER].GetLength()) };
		}
		if (InPipelineDesc->ShaderCode[ESS_HULL_SHADER].GetLength() > 0)
		{
			state.HS = { InPipelineDesc->ShaderCode[ESS_HULL_SHADER].GetBuffer(), uint32(InPipelineDesc->ShaderCode[ESS_HULL_SHADER].GetLength()) };
		}
		if (InPipelineDesc->ShaderCode[ESS_GEOMETRY_SHADER].GetLength() > 0)
		{
			state.GS = { InPipelineDesc->ShaderCode[ESS_GEOMETRY_SHADER].GetBuffer(), uint32(InPipelineDesc->ShaderCode[ESS_GEOMETRY_SHADER].GetLength()) };
		}
		MakeDx12RasterizerDesc(Desc, state.RasterizerState);
		MakeDx12BlendState(Desc, state.BlendState);
		MakeDx12DepthStencilState(Desc, state.DepthStencilState);
		state.SampleMask = UINT_MAX;
		state.PrimitiveTopologyType = k_PRIMITIVE_TYPE_MAP[Desc.PrimitiveType];
		TI_ASSERT(D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED != state.PrimitiveTopologyType);
		state.NumRenderTargets = 1;
		state.RTVFormats[0] = k_PIXEL_FORMAT_MAP[Desc.RTFormats[0]];
		state.DSVFormat = k_PIXEL_FORMAT_MAP[Desc.DepthFormat];
		TI_ASSERT(DXGI_FORMAT_UNKNOWN != state.RTVFormats[0] && DXGI_FORMAT_UNKNOWN != state.DSVFormat);
		state.SampleDesc.Count = 1;

		VALIDATE_HRESULT(D3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&(PipelineDx12->PipelineState))));

		// Shader data can be deleted once the pipeline state is created.
		//for (int32 s = 0; s < ESS_COUNT; ++s)
		//{
		//	InPipelineDesc->.ShaderCode[s].Destroy();
		//}

		return true;
	}
}
#endif	// COMPILE_WITH_RHI_DX12