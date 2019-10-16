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
#include "FArgumentBufferDx12.h"
#include "FGPUCommandSignatureDx12.h"
#include "FGPUCommandBufferDx12.h"
#include <DirectXColors.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>

// link libraries
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "d3dcompiler.lib")

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
		, GraphicsNumBarriersToFlush(0)
		, ComputeNumBarriersToFlush(0)
	{
		// Create frame resource holders
		for (int32 i = 0; i < FRHIConfig::FrameBufferNum; ++i)
		{
			ResHolders[i] = ti_new FFrameResourcesDx12;
			FrameResources[i] = ResHolders[i];
			FenceValues[i] = 0;
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

		// Create the command queue. Graphics and Compute
		D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
		QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		VALIDATE_HRESULT(D3dDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&GraphicsCommandQueue)));
		GraphicsCommandQueue->SetName(L"GraphicsCommandQueue");

		D3D12_COMMAND_QUEUE_DESC ComputeQueueDesc = {};
		ComputeQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		ComputeQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;

		VALIDATE_HRESULT(D3dDevice->CreateCommandQueue(&ComputeQueueDesc, IID_PPV_ARGS(&ComputeCommandQueue)));
		ComputeCommandQueue->SetName(L"ComputeCommandQueue");

		// Create descriptor heaps for render target views and depth stencil views.
		DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].Create(this, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].Create(this, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		
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

		// Pre-allocate memory to avoid alloc in runtime
		GraphicsCommandLists.reserve(16);
		ComputeCommandLists.reserve(16);
		// Create default command list
		DefaultGraphicsCommandList.Create(D3dDevice, EPL_GRAPHICS, 0);
		GraphicsCommandLists.resize(1);
		GraphicsCommandLists[0] = DefaultGraphicsCommandList;
		
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
				GraphicsCommandQueue.Get(),								// Swap chains need a reference to the command queue in DirectX 12.
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

			BackBufferDescriptorTable = CreateRenderResourceTable(FRHIConfig::FrameBufferNum, EHT_RENDERTARGET);
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

			DepthStencilDescriptorTable = CreateRenderResourceTable(1, EHT_DEPTHSTENCIL);
			DepthStencilDescriptor = GetCpuDescriptorHandle(EHT_DEPTHSTENCIL, DepthStencilDescriptorTable->GetStartIndex());
			D3dDevice->CreateDepthStencilView(DepthStencil.Get(), &dsvDesc, DepthStencilDescriptor);
		}
		
		// Set the 3D rendering viewport to target the entire window.
		FRHI::SetViewport(FViewport(0, 0, BackBufferWidth, BackBufferHeight));
	}

	void FRHIDx12::BeginFrame()
	{
		FRHI::BeginFrame();

		CurrentWorkingCommandList = nullptr;
		TI_ASSERT(GraphicsNumBarriersToFlush == 0);
		TI_ASSERT(ComputeNumBarriersToFlush == 0);
		FrameFence = nullptr;

		InitGraphicsPipeline();
	}

	void FRHIDx12::BeginRenderToFrameBuffer()
	{
		TI_ASSERT(CurrentCommandListState.ListType == EPL_GRAPHICS);
		// Start render to frame buffer.
		// Indicate this resource will be in use as a render target.
		Transition(BackBufferRTs[CurrentFrame].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		FlushGraphicsBarriers(CurrentWorkingCommandList.Get());

		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = BackBufferDescriptors[CurrentFrame];
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilDescriptor;
		CurrentWorkingCommandList->ClearRenderTargetView(renderTargetView, DirectX::Colors::CornflowerBlue, 0, nullptr);
		CurrentWorkingCommandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		CurrentWorkingCommandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);
	}

	void FRHIDx12::EndFrame()
	{
		TI_ASSERT(CurrentCommandListState.ListType == EPL_GRAPHICS);
		TI_ASSERT(CurrentWorkingCommandList != nullptr);
		// Indicate that the render target will now be used to present when the command list is done executing.
		Transition(BackBufferRTs[CurrentFrame].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		FlushGraphicsBarriers(CurrentWorkingCommandList.Get());

		VALIDATE_HRESULT(CurrentWorkingCommandList->Close());
		TI_ASSERT(FrameFence == nullptr);
		// Execute the command list by ListExecuteOrder.
		for (uint32 i = 0; i < ListExecuteOrder.size(); ++i)
		{
			const FCommandListState& State = ListExecuteOrder[i];
			
			if (State.ListType == EPL_GRAPHICS)
			{
				if (FrameFence != nullptr)
				{
					GraphicsCommandQueue->Wait(FrameFence.Get(), FenceValues[CurrentFrame]);
				}
				FCommandListDx12& CommandList = GraphicsCommandLists[State.ListIndex];
				ID3D12CommandList* ppCommandLists[] = { CommandList.CommandList.Get() };
				GraphicsCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
				GraphicsCommandQueue->Signal(CommandList.Fence.Get(), FenceValues[CurrentFrame]);
				FrameFence = CommandList.Fence;
			}
			else
			{
				TI_ASSERT(State.ListType == EPL_COMPUTE);
				if (FrameFence != nullptr)
				{
					ComputeCommandQueue->Wait(FrameFence.Get(), FenceValues[CurrentFrame]);
				}
				FCommandListDx12& CommandList = ComputeCommandLists[State.ListIndex];
				ID3D12CommandList* ppCommandLists[] = { CommandList.CommandList.Get() };
				ComputeCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
				ComputeCommandQueue->Signal(CommandList.Fence.Get(), FenceValues[CurrentFrame]);
				FrameFence = CommandList.Fence;
			}
		}

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

	void FRHIDx12::InitGraphicsPipeline()
	{
		TVector<FCommandListDx12>& Lists = GraphicsCommandLists;
		// Close last command list
		if (CurrentWorkingCommandList != nullptr)
		{
			VALIDATE_HRESULT(CurrentWorkingCommandList->Close());
		}

		// Use a new command list
		CurrentCommandListCounter[EPL_GRAPHICS] ++;
		if (CurrentCommandListCounter[EPL_GRAPHICS] >= (int32)Lists.size())
		{
			int32 Index = CurrentCommandListCounter[EPL_GRAPHICS];
			Lists.push_back(FCommandListDx12());
			Lists[Index].Create(D3dDevice, EPL_GRAPHICS, Index);
		}

		// Remember the list we are using, and push it to order vector
		CurrentCommandListState.ListType = EPL_GRAPHICS;
		CurrentCommandListState.ListIndex = CurrentCommandListCounter[EPL_GRAPHICS];
		ListExecuteOrder.push_back(CurrentCommandListState);

		// Grab current command list.
		FCommandListDx12& List = Lists[CurrentCommandListCounter[EPL_GRAPHICS]];
		CurrentWorkingCommandList = List.CommandList;

		// Reset command list
		VALIDATE_HRESULT(List.Allocators[CurrentFrame]->Reset());
		VALIDATE_HRESULT(CurrentWorkingCommandList->Reset(List.Allocators[CurrentFrame].Get(), nullptr));

		// Set the descriptor heaps to be used by this frame.
		ID3D12DescriptorHeap* ppHeaps[] = { DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetHeap() };
		CurrentWorkingCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		// Set the viewport and scissor rectangle.
		D3D12_VIEWPORT ViewportDx = { float(Viewport.Left), float(Viewport.Top), float(Viewport.Width), float(Viewport.Height), 0.f, 1.f };
		CurrentWorkingCommandList->RSSetViewports(1, &ViewportDx);
		D3D12_RECT ScissorRect = { Viewport.Left, Viewport.Top, Viewport.Width, Viewport.Height };
		CurrentWorkingCommandList->RSSetScissorRects(1, &ScissorRect);
	}

	void FRHIDx12::BeginComputeTask(bool IsTileComputeShader)
	{
		// Switch from graphics command list to compute command list.
		TI_ASSERT(CurrentCommandListState.ListType == EPL_GRAPHICS);
		TVector<FCommandListDx12>& Lists = ComputeCommandLists;
		// Close last command list
		if (CurrentWorkingCommandList != nullptr)
		{
			VALIDATE_HRESULT(CurrentWorkingCommandList->Close());
		}

		// Use a new command list
		CurrentCommandListCounter[EPL_COMPUTE] ++;
		if (CurrentCommandListCounter[EPL_COMPUTE] >= (int32)Lists.size())
		{
			int32 Index = CurrentCommandListCounter[EPL_COMPUTE];
			Lists.push_back(FCommandListDx12());
			Lists[Index].Create(D3dDevice, EPL_COMPUTE, Index);
		}

		// Remember the list we are using, and push it to order vector
		CurrentCommandListState.ListType = EPL_COMPUTE;
		CurrentCommandListState.ListIndex = CurrentCommandListCounter[EPL_COMPUTE];
		ListExecuteOrder.push_back(CurrentCommandListState);

		// Grab current command list.
		FCommandListDx12& List = Lists[CurrentCommandListCounter[EPL_COMPUTE]];
		CurrentWorkingCommandList = List.CommandList;

		// Reset command list
		VALIDATE_HRESULT(List.Allocators[CurrentFrame]->Reset());
		VALIDATE_HRESULT(CurrentWorkingCommandList->Reset(List.Allocators[CurrentFrame].Get(), nullptr));

		// Set the descriptor heaps to be used by this frame.
		ID3D12DescriptorHeap* ppHeaps[] = { DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetHeap() };
		CurrentWorkingCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	}

	// No use of param ComputeTask, it's for metal.
	void FRHIDx12::EndComputeTask(bool IsTileComputeShader)
	{
		// Switch from compute command list to graphics command list.
		TI_ASSERT(CurrentCommandListState.ListType == EPL_COMPUTE);
		InitGraphicsPipeline();
	}

	FTexturePtr FRHIDx12::CreateTexture()
	{
		return ti_new FTextureDx12();
	}

	FTexturePtr FRHIDx12::CreateTexture(const TTextureDesc& Desc)
	{
		return ti_new FTextureDx12(Desc);
	}

	FUniformBufferPtr FRHIDx12::CreateUniformBuffer(uint32 InStructureSizeInBytes, uint32 Elements, uint32 Flag)
	{
		if ((Flag & UB_FLAG_READBACK) != 0)
		{
			return ti_new FUniformBufferReadableDx12(InStructureSizeInBytes, Elements, Flag);
		}
		else
		{
			return ti_new FUniformBufferDx12(InStructureSizeInBytes, Elements, Flag);
		}
	}

	FMeshBufferPtr FRHIDx12::CreateMeshBuffer()
	{
		return ti_new FMeshBufferDx12();
	}

	FMeshBufferPtr FRHIDx12::CreateEmptyMeshBuffer(
		E_PRIMITIVE_TYPE InPrimType,
		uint32 InVSFormat,
		uint32 InVertexCount,
		E_INDEX_TYPE InIndexType,
		uint32 InIndexCount
	)
	{
		return ti_new FMeshBufferDx12(InPrimType, InVSFormat, InVertexCount, InIndexType, InIndexCount);
	}

	FInstanceBufferPtr FRHIDx12::CreateInstanceBuffer()
	{
		return ti_new FInstanceBufferDx12();
	}

	FInstanceBufferPtr FRHIDx12::CreateEmptyInstanceBuffer(uint32 InstanceCount, uint32 InstanceStride)
	{
		return ti_new FInstanceBufferDx12(InstanceCount, InstanceStride);
	}

	FPipelinePtr FRHIDx12::CreatePipeline(FShaderPtr InShader)
	{
		return ti_new FPipelineDx12(InShader);
	}

	FRenderTargetPtr FRHIDx12::CreateRenderTarget(int32 W, int32 H)
	{
		return ti_new FRenderTargetDx12(W, H);
	}

	FShaderPtr FRHIDx12::CreateShader(const TShaderNames& InNames)
	{
		return ti_new FShaderDx12(InNames);
	}

	FShaderPtr FRHIDx12::CreateComputeShader(const TString& ComputeShaderName)
	{
		return ti_new FShaderDx12(ComputeShaderName);
	}

	FArgumentBufferPtr FRHIDx12::CreateArgumentBuffer(int32 ReservedSlots)
	{
		return ti_new FArgumentBufferDx12(ReservedSlots);
	}

	FGPUCommandSignaturePtr FRHIDx12::CreateGPUCommandSignature(FPipelinePtr Pipeline, const TVector<E_GPU_COMMAND_TYPE>& CommandStructure)
	{
		return ti_new FGPUCommandSignatureDx12(Pipeline, CommandStructure);
	}

	FGPUCommandBufferPtr FRHIDx12::CreateGPUCommandBuffer(FGPUCommandSignaturePtr GPUCommandSignature, uint32 CommandsCount, uint32 Flag)
	{
		return ti_new FGPUCommandBufferDx12(GPUCommandSignature, CommandsCount, Flag);
	}

	int32 FRHIDx12::GetCurrentEncodingFrameIndex()
	{
		return CurrentFrame;
	}

	// Wait for pending GPU work to complete.
	void FRHIDx12::WaitingForGpu()
	{
		// Frame fence is the last fence of current frame
		TI_ASSERT(FrameFence != nullptr);
		// Schedule a Signal command in the queue.
		VALIDATE_HRESULT(GraphicsCommandQueue->Signal(FrameFence.Get(), FenceValues[CurrentFrame]));

		// Wait until the fence has been crossed.
		VALIDATE_HRESULT(FrameFence->SetEventOnCompletion(FenceValues[CurrentFrame], FenceEvent));
		WaitForSingleObjectEx(FenceEvent, INFINITE, FALSE);

		// Increment the fence value for the current frame.
		FenceValues[CurrentFrame]++;
	}

	// Prepare to render the next frame.
	void FRHIDx12::MoveToNextFrame()
	{
		// Schedule a Signal command in the queue.
		const uint64 currentFenceValue = FenceValues[CurrentFrame];
		TI_ASSERT(CurrentCommandListState.ListType == EPL_GRAPHICS);
		VALIDATE_HRESULT(GraphicsCommandQueue->Signal(FrameFence.Get(), currentFenceValue));

		// Advance the frame index.
		CurrentFrame = SwapChain->GetCurrentBackBufferIndex();

		// Check to see if the next frame is ready to start.
		if (FrameFence->GetCompletedValue() < FenceValues[CurrentFrame])
		{
			VALIDATE_HRESULT(FrameFence->SetEventOnCompletion(FenceValues[CurrentFrame], FenceEvent));
			WaitForSingleObjectEx(FenceEvent, INFINITE, FALSE);
			FRHI::GPUFrameDone();
		}

		// Set the fence value for the next frame.
		FenceValues[CurrentFrame] = currentFenceValue + 1;

		// Release resources references for next drawing
		FrameResources[CurrentFrame]->RemoveAllReferences();
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
			Transition(GPUResource->GetResource().Get(), GPUResource->UsageState, stateAfter, subresource, flags);
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
		TI_ASSERT(pResource != nullptr);
		D3D12_RESOURCE_BARRIER& barrier = GraphicsBarrierBuffers[GraphicsNumBarriersToFlush++];
		TI_ASSERT(GraphicsNumBarriersToFlush <= MaxResourceBarrierBuffers);
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = flags;
		barrier.Transition.pResource = pResource;
		barrier.Transition.StateBefore = stateBefore;
		barrier.Transition.StateAfter = stateAfter;
		barrier.Transition.Subresource = subresource;
	}

	void FRHIDx12::FlushGraphicsBarriers(
		_In_ ID3D12GraphicsCommandList* pCmdList)
	{
		if (GraphicsNumBarriersToFlush > 0)
		{
			pCmdList->ResourceBarrier(GraphicsNumBarriersToFlush, GraphicsBarrierBuffers);
			GraphicsNumBarriersToFlush = 0;
		}
	}

	bool FRHIDx12::UpdateHardwareResourceMesh(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData)
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

			UpdateSubresources(CurrentWorkingCommandList.Get(), MBDx12->VertexBuffer.Get(), VertexBufferUpload.Get(), 0, 0, 1, &VertexData);

			if (MeshBuffer->GetUsage() == FRenderResource::USAGE_DEFAULT)
			{
				Transition(MBDx12->VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			}
			else if (MeshBuffer->GetUsage() == FRenderResource::USAGE_COPY_SOURCE)
			{
				Transition(MBDx12->VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
			}
			else
			{
				TI_ASSERT(0);
			}
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

			UpdateSubresources(CurrentWorkingCommandList.Get(), MBDx12->IndexBuffer.Get(), IndexBufferUpload.Get(), 0, 0, 1, &IndexData);

			if (MeshBuffer->GetUsage() == FRenderResource::USAGE_DEFAULT)
			{
				Transition(MBDx12->IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
			}
			else if (MeshBuffer->GetUsage() == FRenderResource::USAGE_COPY_SOURCE)
			{
				Transition(MBDx12->IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
			}
			else
			{
				TI_ASSERT(0);
			}
		}

		FlushGraphicsBarriers(CurrentWorkingCommandList.Get());

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

	bool FRHIDx12::UpdateHardwareResourceMesh(
		FMeshBufferPtr MeshBuffer, 
		uint32 VertexDataSize, 
		uint32 VertexDataStride, 
		uint32 IndexDataSize,
		E_INDEX_TYPE IndexType, 
		const TString& BufferName)
	{
#if defined (TIX_DEBUG)
		MeshBuffer->SetResourceName(BufferName);
#endif
		FMeshBufferDx12 * MBDx12 = static_cast<FMeshBufferDx12*>(MeshBuffer.get());

		// Create empty vertex buffer resource in the GPU's default heap
		const int32 BufferSize = VertexDataSize;
		CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(BufferSize);
		VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&vertexBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&MBDx12->VertexBuffer)));

		DX_SETNAME(MBDx12->VertexBuffer.Get(), BufferName + "-VB");

		// Create vertex buffer views.
		MBDx12->VertexBufferView.BufferLocation = MBDx12->VertexBuffer->GetGPUVirtualAddress();
		MBDx12->VertexBufferView.StrideInBytes = VertexDataStride;
		MBDx12->VertexBufferView.SizeInBytes = BufferSize;

		// Create empty index buffer resource.
		if (IndexDataSize > 0)
		{
			const uint32 IndexBufferSize = IndexDataSize;

			// Create the index buffer resource in the GPU's default heap 
			CD3DX12_RESOURCE_DESC IndexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize);
			VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
				&defaultHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&IndexBufferDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&MBDx12->IndexBuffer)));

			DX_SETNAME(MBDx12->IndexBuffer.Get(), BufferName + "-ib");

			// Create index buffer views.
			MBDx12->IndexBufferView.BufferLocation = MBDx12->IndexBuffer->GetGPUVirtualAddress();
			MBDx12->IndexBufferView.SizeInBytes = IndexBufferSize;
			MBDx12->IndexBufferView.Format = IndexType == EIT_16BIT ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		}
		// Hold resources used here
		HoldResourceReference(MeshBuffer);

		return true;
	}

	bool FRHIDx12::UpdateHardwareResourceIB(FInstanceBufferPtr InstanceBuffer, TInstanceBufferPtr InInstanceData)
	{
#if defined (TIX_DEBUG)
		if (InInstanceData != nullptr)
			InstanceBuffer->SetResourceName(InInstanceData->GetResourceName());
#endif
		FInstanceBufferDx12 * InsDx12 = static_cast<FInstanceBufferDx12*>(InstanceBuffer.get());

		// Create the vertex buffer resource in the GPU's default heap and copy vertex data into it using the upload heap.
		// The upload resource must not be released until after the GPU has finished using it.
		
		const int32 BufferSize = InstanceBuffer->GetInstancesCount() * InstanceBuffer->GetStride();
		TI_ASSERT(BufferSize > 0);
		CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(BufferSize);
		VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&vertexBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&InsDx12->InstanceBuffer)));

		if (InInstanceData != nullptr)
		{
			ComPtr<ID3D12Resource> VertexBufferUpload;
			CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
			VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
				&uploadHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&vertexBufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&VertexBufferUpload)));

			DX_SETNAME(InsDx12->InstanceBuffer.Get(), InstanceBuffer->GetResourceName() + "-INSB");

			// Upload the instance buffer to the GPU.
			{
				D3D12_SUBRESOURCE_DATA VertexData = {};
				VertexData.pData = reinterpret_cast<const uint8*>(InInstanceData->GetInstanceData());
				VertexData.RowPitch = BufferSize;
				VertexData.SlicePitch = VertexData.RowPitch;

				UpdateSubresources(CurrentWorkingCommandList.Get(), InsDx12->InstanceBuffer.Get(), VertexBufferUpload.Get(), 0, 0, 1, &VertexData);

				if (InstanceBuffer->GetUsage() == FRenderResource::USAGE_DEFAULT)
				{
					Transition(InsDx12->InstanceBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
				}
				else if (InstanceBuffer->GetUsage() == FRenderResource::USAGE_COPY_SOURCE)
				{
					Transition(InsDx12->InstanceBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
				}
				else
				{
					TI_ASSERT(0);
				}
			}

			FlushGraphicsBarriers(CurrentWorkingCommandList.Get());
			HoldResourceReference(VertexBufferUpload);
		}


		// Create vertex/index buffer views.
		InsDx12->InstanceBufferView.BufferLocation = InsDx12->InstanceBuffer->GetGPUVirtualAddress();
		InsDx12->InstanceBufferView.StrideInBytes = InstanceBuffer->GetStride();
		InsDx12->InstanceBufferView.SizeInBytes = BufferSize;

		// Hold resources used here
		HoldResourceReference(InstanceBuffer);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Returns required size of a buffer to be used for data upload
	inline uint64 GetRequiredIntermediateSize(
		_In_ ID3D12Device* D3dDevice,
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

#if defined (TIX_DEBUG)
		uint64 CompareSize;
		D3dDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, 0, nullptr, nullptr, nullptr, &CompareSize);
		TI_ASSERT(CompareSize == RequiredSize);
#endif

		return RequiredSize;
	}

	inline uint64 GetRequiredIntermediateSize(
		_In_ ID3D12Device* D3dDevice,
		const D3D12_RESOURCE_DESC& Desc,
		_In_range_(0, D3D12_REQ_SUBRESOURCES) uint32 FirstSubresource,
		_In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) uint32 NumSubresources)
	{
		uint64 RequiredSize = 0;
		D3dDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, 0, nullptr, nullptr, nullptr, &RequiredSize);

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
	bool FRHIDx12::UpdateHardwareResourceTexture(FTexturePtr Texture)
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

		D3D12_CLEAR_VALUE *pOptimizedClearValue = nullptr;
		if ((TextureDx12Desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) != 0)
		{
			pOptimizedClearValue = &ClearValue;
		}

		TexDx12->TextureResource.CreateResource(
			D3dDevice.Get(),
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&TextureDx12Desc,
			D3D12_RESOURCE_STATE_COMMON,
			pOptimizedClearValue
		);
		DX_SETNAME(TexDx12->TextureResource.GetResource().Get(), Texture->GetResourceName());

		Texture->SetTextureFlag(ETF_RENDER_RESOURCE_UPDATED, true);
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

	bool FRHIDx12::UpdateHardwareResourceTexture(FTexturePtr Texture, TTexturePtr InTexData)
	{
		TI_ASSERT(InTexData != nullptr);
		FTextureDx12 * TexDx12 = static_cast<FTextureDx12*>(Texture.get());
		const TTextureDesc& Desc = TexDx12->GetDesc();
		DXGI_FORMAT DxgiFormat = GetDxPixelFormat(Desc.Format);
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

		if (!TexDx12->TextureResource.IsInited())
		{
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
		}
		else
		{
			TI_ASSERT(Desc.Width == InTexData->GetDesc().Width && Desc.Height == InTexData->GetDesc().Height);
		}

		const int32 SubResourceNum = ArraySize * Desc.Mips;
		const uint64 uploadBufferSize = GetRequiredIntermediateSize(D3dDevice.Get(), TexDx12->TextureResource.GetResource().Get(), 0, SubResourceNum);

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

		UpdateSubresources(CurrentWorkingCommandList.Get(), TexDx12->TextureResource.GetResource().Get(), TextureUploadHeap.Get(), 0, 0, SubResourceNum, TextureDatas);
		Transition(&TexDx12->TextureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		DX_SETNAME(TexDx12->TextureResource.GetResource().Get(), Texture->GetResourceName());

		ti_delete[] TextureDatas;

		Texture->SetTextureFlag(ETF_RENDER_RESOURCE_UPDATED, true);

		FlushGraphicsBarriers(CurrentWorkingCommandList.Get());
		// Hold resources used here
		HoldResourceReference(Texture);
		HoldResourceReference(TextureUploadHeap);

		return true;
	}

	bool FRHIDx12::UpdateHardwareResourceTexture(FTexturePtr Texture, TImagePtr InImageData)
	{
		TI_ASSERT(InImageData != nullptr);
		FTextureDx12 * TexDx12 = static_cast<FTextureDx12*>(Texture.get());
		const TTextureDesc& Desc = TexDx12->GetDesc();
		DXGI_FORMAT DxgiFormat = GetDxPixelFormat(Desc.Format);
		const bool IsCubeMap = Desc.Type == ETT_TEXTURE_CUBE;

		// Create texture resource and fill with texture data.

		// Note: ComPtr's are CPU objects but this resource needs to stay in scope until
		// the command list that references it has finished executing on the GPU.
		// We will flush the GPU at the end of this method to ensure the resource is not
		// prematurely destroyed.
		ComPtr<ID3D12Resource> TextureUploadHeap;
		const int32 ArraySize = 1;

		if (!TexDx12->TextureResource.IsInited())
		{
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
		}
		else
		{
			TI_ASSERT(Desc.Width == InImageData->GetWidth() && Desc.Height == InImageData->GetHeight());
		}
		Transition(&TexDx12->TextureResource, D3D12_RESOURCE_STATE_COPY_DEST);
		FlushGraphicsBarriers(CurrentWorkingCommandList.Get());

		const int32 SubResourceNum = ArraySize * Desc.Mips;
		const uint64 uploadBufferSize = GetRequiredIntermediateSize(D3dDevice.Get(), TexDx12->TextureResource.GetResource().Get(), 0, SubResourceNum);

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
		TI_ASSERT(SubResourceNum == InImageData->GetMipmapCount());
		for (int32 s = 0; s < SubResourceNum; ++s)
		{
			D3D12_SUBRESOURCE_DATA& texData = TextureDatas[s];
			const TImage::TSurfaceData& MipSurface = InImageData->GetMipmap(s);
			texData.pData = MipSurface.Data.GetBuffer();
			texData.RowPitch = MipSurface.RowPitch;
			texData.SlicePitch = MipSurface.Data.GetLength();
		}

		UpdateSubresources(CurrentWorkingCommandList.Get(), TexDx12->TextureResource.GetResource().Get(), TextureUploadHeap.Get(), 0, 0, SubResourceNum, TextureDatas);
		Transition(&TexDx12->TextureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		DX_SETNAME(TexDx12->TextureResource.GetResource().Get(), Texture->GetResourceName());

		ti_delete[] TextureDatas;

		Texture->SetTextureFlag(ETF_RENDER_RESOURCE_UPDATED, true);

		FlushGraphicsBarriers(CurrentWorkingCommandList.Get());
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

	bool FRHIDx12::UpdateHardwareResourcePL(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc)
	{
		FPipelineDx12 * PipelineDx12 = static_cast<FPipelineDx12*>(Pipeline.get());
		FShaderPtr Shader = Pipeline->GetShader();

		if (Shader->GetShaderType() == EST_RENDER)
		{
#if defined (TIX_DEBUG)
			Pipeline->SetResourceName(InPipelineDesc->GetResourceName());
#endif
			TI_ASSERT(InPipelineDesc != nullptr);
			const TPipelineDesc& Desc = InPipelineDesc->GetDesc();
			TI_ASSERT(Shader == Desc.Shader->ShaderResource);

			TVector<E_MESH_STREAM_INDEX> VertexStreams = TMeshBuffer::GetSteamsFromFormat(Desc.VsFormat);
			TVector<E_INSTANCE_STREAM_INDEX> InstanceStreams = TInstanceBuffer::GetSteamsFromFormat(Desc.InsFormat);
			TVector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
			InputLayout.resize(VertexStreams.size() + InstanceStreams.size());

			// Fill layout desc
			uint32 VertexDataOffset = 0;
			for (uint32 i = 0; i < VertexStreams.size(); ++i)
			{
				E_MESH_STREAM_INDEX Stream = VertexStreams[i];
				D3D12_INPUT_ELEMENT_DESC& InputElement = InputLayout[i];
				InputElement.SemanticName = TMeshBuffer::SemanticName[Stream];
				InputElement.SemanticIndex = TMeshBuffer::SemanticIndex[Stream];
				InputElement.Format = k_MESHBUFFER_STREAM_FORMAT_MAP[Stream];
				InputElement.InputSlot = 0;
				InputElement.AlignedByteOffset = VertexDataOffset;
				VertexDataOffset += TMeshBuffer::SemanticSize[Stream];
				InputElement.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				InputElement.InstanceDataStepRate = 0;
			}
			uint32 InstanceDataOffset = 0;
			for (uint32 i = 0; i < InstanceStreams.size(); ++i)
			{
				E_INSTANCE_STREAM_INDEX Stream = InstanceStreams[i];
				D3D12_INPUT_ELEMENT_DESC& InputElement = InputLayout[VertexStreams.size() + i];
				InputElement.SemanticName = TInstanceBuffer::SemanticName[Stream];
				InputElement.SemanticIndex = TInstanceBuffer::SemanticIndex[Stream];
				InputElement.Format = k_INSTANCEBUFFER_STREAM_FORMAT_MAP[Stream];
				InputElement.InputSlot = 1;
				InputElement.AlignedByteOffset = InstanceDataOffset;
				InstanceDataOffset += TInstanceBuffer::SemanticSize[Stream];
				InputElement.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
				InputElement.InstanceDataStepRate = 1;
			}

			FShaderDx12 * ShaderDx12 = static_cast<FShaderDx12*>(Shader.get());
			FShaderBindingPtr Binding = ShaderDx12->ShaderBinding;
			TI_ASSERT(Binding != nullptr);
			FRootSignatureDx12 * PipelineRS = static_cast<FRootSignatureDx12*>(Binding.get());

			D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
			state.InputLayout = { &(InputLayout[0]), uint32(InputLayout.size()) };
			state.pRootSignature = PipelineRS->Get();

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
			for (int32 r = 0; r < Desc.RTCount; ++r)
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
		}
		else
		{
			// Compute pipeline 
			FShaderDx12 * ShaderDx12 = static_cast<FShaderDx12*>(Shader.get());
			FShaderBindingPtr Binding = ShaderDx12->ShaderBinding;
			TI_ASSERT(Binding != nullptr);
			FRootSignatureDx12 * PipelineRS = static_cast<FRootSignatureDx12*>(Binding.get());

			D3D12_COMPUTE_PIPELINE_STATE_DESC state = {};
			state.pRootSignature = PipelineRS->Get();
			state.CS = { ShaderDx12->ShaderCodes[0].GetBuffer(), uint32(ShaderDx12->ShaderCodes[0].GetLength()) };

			VALIDATE_HRESULT(D3dDevice->CreateComputePipelineState(&state, IID_PPV_ARGS(&(PipelineDx12->PipelineState))));

			// Shader data can be deleted once the pipeline state is created.
			ShaderDx12->ReleaseShaderCode();
		}
		HoldResourceReference(Pipeline);

		return true;
	}

	bool FRHIDx12::UpdateHardwareResourceTilePL(FPipelinePtr Pipeline, TTilePipelinePtr InTilePipelineDesc)
	{
		// This is for Metal
		TI_ASSERT(0);
		return false;
	}

	static const int32 UniformBufferAlignSize = 256;
	bool FRHIDx12::UpdateHardwareResourceUB(FUniformBufferPtr UniformBuffer, const void* InData)
	{
		FUniformBufferDx12 * UniformBufferDx12 = static_cast<FUniformBufferDx12*>(UniformBuffer.get());
		
		if ((UniformBuffer->GetFlag() & UB_FLAG_COMPUTE_WRITABLE) != 0)
		{
			// Add a counter for UAV
			int32 BufferSize = UniformBuffer->GetTotalBufferSize();
			if ((UniformBuffer->GetFlag() & UB_FLAG_COMPUTE_WITH_COUNTER) != 0)
			{
				// With counter, counter offset must be aligned with D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT
				BufferSize = FUniformBufferDx12::AlignForUavCounter(BufferSize);
				BufferSize += sizeof(uint32);
			}
			CD3DX12_RESOURCE_DESC BufferDesc = CD3DX12_RESOURCE_DESC::Buffer(BufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

			UniformBufferDx12->BufferResource.CreateResource(
				D3dDevice.Get(),
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&BufferDesc,
				D3D12_RESOURCE_STATE_COPY_DEST);

			//Transition(&UniformBufferDx12->BufferResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			//FlushGraphicsBarriers(ComputeCommandList.Get());
		}
		else
		{
			const int32 AlignedDataSize = ti_align(UniformBuffer->GetTotalBufferSize(), UniformBufferAlignSize);
			CD3DX12_RESOURCE_DESC ConstantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(AlignedDataSize);

			if ((UniformBuffer->GetFlag() & UB_FLAG_INTERMEDIATE) != 0)
			{
				TI_ASSERT(InData != nullptr);

				CD3DX12_HEAP_PROPERTIES UploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
				UniformBufferDx12->BufferResource.CreateResource(
					D3dDevice.Get(),
					&UploadHeapProperties,
					D3D12_HEAP_FLAG_NONE,
					&ConstantBufferDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ);

				// Map the constant buffers.
				uint8 * MappedConstantBuffer = nullptr;
				CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
				VALIDATE_HRESULT(UniformBufferDx12->BufferResource.GetResource()->Map(0, &readRange, reinterpret_cast<void**>(&MappedConstantBuffer)));
				memcpy(MappedConstantBuffer, InData, UniformBuffer->GetTotalBufferSize());
				if (AlignedDataSize - UniformBuffer->GetTotalBufferSize() > 0)
				{
					memset(MappedConstantBuffer + UniformBuffer->GetTotalBufferSize(), 0, AlignedDataSize - UniformBuffer->GetTotalBufferSize());
				}
				UniformBufferDx12->BufferResource.GetResource()->Unmap(0, nullptr);
			}
			else
			{
				CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
				UniformBufferDx12->BufferResource.CreateResource(
					D3dDevice.Get(),
					&defaultHeapProperties,
					D3D12_HEAP_FLAG_NONE,
					&ConstantBufferDesc,
					D3D12_RESOURCE_STATE_COPY_DEST);

				if (InData != nullptr)
				{
					ComPtr<ID3D12Resource> UniformBufferUpload;
					CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
					VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
						&uploadHeapProperties,
						D3D12_HEAP_FLAG_NONE,
						&ConstantBufferDesc,
						D3D12_RESOURCE_STATE_GENERIC_READ,
						nullptr,
						IID_PPV_ARGS(&UniformBufferUpload)));

					// Upload the index buffer to the GPU.
					{
						D3D12_SUBRESOURCE_DATA UniformData = {};
						UniformData.pData = reinterpret_cast<const uint8*>(InData);
						UniformData.RowPitch = UniformBuffer->GetTotalBufferSize();
						UniformData.SlicePitch = UniformData.RowPitch;

						UpdateSubresources(CurrentWorkingCommandList.Get(), UniformBufferDx12->BufferResource.GetResource().Get(), UniformBufferUpload.Get(), 0, 0, 1, &UniformData);

						if ((UniformBuffer->GetFlag() & UB_FLAG_GPU_COMMAND_BUFFER) != 0)
						{
							Transition(UniformBufferDx12->BufferResource.GetResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
						}
						else if ((UniformBuffer->GetFlag() & UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE) != 0)
						{
							Transition(UniformBufferDx12->BufferResource.GetResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
						}
						else
						{
							Transition(UniformBufferDx12->BufferResource.GetResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
						}
					}
					FlushGraphicsBarriers(CurrentWorkingCommandList.Get());
					HoldResourceReference(UniformBufferUpload);
				}
			}
		}

		HoldResourceReference(UniformBuffer);

		return true;
	}

	void FRHIDx12::PrepareDataForCPU(FUniformBufferPtr UniformBuffer)
	{
		if ((UniformBuffer->GetFlag() & UB_FLAG_READBACK) != 0)
		{
			FUniformBufferReadableDx12 * UniformBufferDx12 = static_cast<FUniformBufferReadableDx12*>(UniformBuffer.get());
			if (UniformBufferDx12->ReadbackResource.GetResource() == nullptr)
			{
				int32 BufferSize = UniformBuffer->GetTotalBufferSize();
				D3D12_HEAP_PROPERTIES ReadbackHeapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK) };
				D3D12_RESOURCE_DESC ReadbackBufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(BufferSize) };

				UniformBufferDx12->ReadbackResource.CreateResource(
					D3dDevice.Get(),
					&ReadbackHeapProperties,
					D3D12_HEAP_FLAG_NONE,
					&ReadbackBufferDesc,
					D3D12_RESOURCE_STATE_COPY_DEST);
			}
			
			{
				// Transition the status of uniform from D3D12_RESOURCE_STATE_COPY_DEST to D3D12_RESOURCE_STATE_COPY_SOURCE
				TI_ASSERT(UniformBufferDx12->BufferResource.GetCurrentState() == D3D12_RESOURCE_STATE_COPY_DEST);
				Transition(&UniformBufferDx12->BufferResource, D3D12_RESOURCE_STATE_COPY_SOURCE);
				FlushGraphicsBarriers(CurrentWorkingCommandList.Get());
			}

			CurrentWorkingCommandList->CopyResource(UniformBufferDx12->ReadbackResource.GetResource().Get(), UniformBufferDx12->BufferResource.GetResource().Get());
			HoldResourceReference(UniformBuffer);

			// Code goes here to close, execute (and optionally reset) the command list, and also
			// to use a fence to wait for the command queue.
		}
		else
		{
			_LOG(Error, "Can not read buffer without flag UB_FLAG_READBACK.\n");
		}
	}

	bool FRHIDx12::CopyTextureRegion(FTexturePtr DstTexture, const recti& InDstRegion, FTexturePtr SrcTexture)
	{
		TI_ASSERT(SrcTexture != nullptr);
		TI_ASSERT(SrcTexture->GetDesc().Width == InDstRegion.getWidth() && SrcTexture->GetDesc().Height == InDstRegion.getHeight());
		FTextureDx12 * DstTexDx12 = static_cast<FTextureDx12*>(DstTexture.get());
		FTextureDx12 * SrcTexDx12 = static_cast<FTextureDx12*>(SrcTexture.get());
		TI_ASSERT(DstTexDx12->GetDesc().Type == ETT_TEXTURE_2D && SrcTexDx12->GetDesc().Type == ETT_TEXTURE_2D);
		TI_ASSERT(InDstRegion.getWidth() == SrcTexture->GetDesc().Width && InDstRegion.getHeight() == SrcTexture->GetDesc().Height);
		TI_ASSERT(DstTexture->GetDesc().Format == SrcTexture->GetDesc().Format);

		Transition(&DstTexDx12->TextureResource, D3D12_RESOURCE_STATE_COPY_DEST);
		Transition(&SrcTexDx12->TextureResource, D3D12_RESOURCE_STATE_COPY_SOURCE);
		FlushGraphicsBarriers(CurrentWorkingCommandList.Get());

		D3D12_BOX SrcBox;
		SrcBox.left = 0;
		SrcBox.top = 0;
		SrcBox.front = 0;
		SrcBox.right = InDstRegion.getWidth();
		SrcBox.bottom = InDstRegion.getHeight();
		SrcBox.back = 1;
		CD3DX12_TEXTURE_COPY_LOCATION Dst(DstTexDx12->TextureResource.GetResource().Get(), 0);
		CD3DX12_TEXTURE_COPY_LOCATION Src(SrcTexDx12->TextureResource.GetResource().Get(), 0);
		CurrentWorkingCommandList->CopyTextureRegion(&Dst, InDstRegion.Left, InDstRegion.Upper, 0, &Src, &SrcBox);

		// Hold resources used here
		HoldResourceReference(DstTexture);
		HoldResourceReference(SrcTexture);

		return true;
	}

	bool FRHIDx12::CopyBufferRegion(FUniformBufferPtr DstBuffer, uint32 DstOffset, FUniformBufferPtr SrcBuffer, uint32 Length)
	{
		TI_ASSERT(DstOffset + Length <= DstBuffer->GetTotalBufferSize());
		TI_ASSERT(Length <= SrcBuffer->GetTotalBufferSize());

		FUniformBufferDx12 * DstBufferDx12 = static_cast<FUniformBufferDx12*>(DstBuffer.get());
		FUniformBufferDx12 * SrcBufferDx12 = static_cast<FUniformBufferDx12*>(SrcBuffer.get());

		CurrentWorkingCommandList->CopyBufferRegion(DstBufferDx12->BufferResource.GetResource().Get(), DstOffset, SrcBufferDx12->BufferResource.GetResource().Get(), 0, Length);

		// Hold resources used here
		HoldResourceReference(DstBuffer);
		HoldResourceReference(SrcBuffer);

		return true;
	}

	bool FRHIDx12::CopyBufferRegion(
		FMeshBufferPtr DstBuffer,
		uint32 DstVertexOffset,
		uint32 DstIndexOffset,
		FMeshBufferPtr SrcBuffer,
		uint32 SrcVertexOffset,
		uint32 VertexLength,
		uint32 SrcIndexOffset,
		uint32 IndexLength)
	{
		FMeshBufferDx12 * DstBufferDx12 = static_cast<FMeshBufferDx12*>(DstBuffer.get());
		FMeshBufferDx12 * SrcBufferDx12 = static_cast<FMeshBufferDx12*>(SrcBuffer.get());

		// Copy vertex data
		TI_ASSERT(DstBuffer->GetVSFormat() == SrcBuffer->GetVSFormat());
		TI_ASSERT(DstVertexOffset + VertexLength <= DstBuffer->GetVerticesCount() * DstBuffer->GetStride());
		CurrentWorkingCommandList->CopyBufferRegion(DstBufferDx12->VertexBuffer.Get(), DstVertexOffset, SrcBufferDx12->VertexBuffer.Get(), SrcVertexOffset, VertexLength);

		// Copy index data
		TI_ASSERT(DstBuffer->GetIndexType() == SrcBuffer->GetIndexType());
		TI_ASSERT(DstIndexOffset + IndexLength <= (uint32)(DstBuffer->GetIndicesCount() * (DstBuffer->GetIndexType() == EIT_16BIT ? 2 : 4)));
		CurrentWorkingCommandList->CopyBufferRegion(DstBufferDx12->IndexBuffer.Get(), DstIndexOffset, SrcBufferDx12->IndexBuffer.Get(), SrcIndexOffset, IndexLength);

		HoldResourceReference(DstBuffer);
		HoldResourceReference(SrcBuffer);

		return true;
	}

	bool FRHIDx12::CopyBufferRegion(
		FInstanceBufferPtr DstBuffer,
		uint32 DstInstanceOffset,
		FInstanceBufferPtr SrcBuffer,
		uint32 SrcInstanceOffset,
		uint32 InstanceCount)
	{
		FInstanceBufferDx12 * DstBufferDx12 = static_cast<FInstanceBufferDx12*>(DstBuffer.get());
		FInstanceBufferDx12 * SrcBufferDx12 = static_cast<FInstanceBufferDx12*>(SrcBuffer.get());

		// Copy vertex data
		const uint32 InstanceStride = DstBuffer->GetStride();
		TI_ASSERT(DstBuffer->GetStride() == SrcBuffer->GetStride());
		TI_ASSERT(DstInstanceOffset + InstanceCount <= DstBuffer->GetInstancesCount());
		CurrentWorkingCommandList->CopyBufferRegion(
			DstBufferDx12->InstanceBuffer.Get(), 
			DstInstanceOffset * InstanceStride, 
			SrcBufferDx12->InstanceBuffer.Get(), 
			SrcInstanceOffset * InstanceStride, 
			InstanceCount * InstanceStride);

		HoldResourceReference(DstBuffer);
		HoldResourceReference(SrcBuffer);

		return true;
	}

	void FRHIDx12::SetResourceStateUB(FUniformBufferPtr InUniformBuffer, E_RESOURCE_STATE NewState)
	{
		FUniformBufferReadableDx12 * UniformBufferDx12 = static_cast<FUniformBufferReadableDx12*>(InUniformBuffer.get());
		Transition(&UniformBufferDx12->BufferResource, TiX2DxResourceStateMap[NewState]);
		FlushGraphicsBarriers(CurrentWorkingCommandList.Get());
	}

	bool FRHIDx12::UpdateHardwareResourceRT(FRenderTargetPtr RenderTarget)
	{
		FRenderTargetDx12 * RTDx12 = static_cast<FRenderTargetDx12*>(RenderTarget.get());
		// Create render target render resource tables
		int32 ColorBufferCount = RenderTarget->GetColorBufferCount();
		TI_ASSERT(RTDx12->RTColorTable == nullptr);
		RTDx12->RTColorTable = CreateRenderResourceTable(ColorBufferCount, EHT_RENDERTARGET);
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
				RTDx12->RTDepthTable = CreateRenderResourceTable(1, EHT_DEPTHSTENCIL);
				RTDx12->RTDepthTable->PutRTDepthInTable(DSBufferTexture, 0);
			}
		}
		return true;
	}

	inline int32 GetBindIndex(const D3D12_SHADER_INPUT_BIND_DESC& BindDesc, const D3D12_ROOT_SIGNATURE_DESC& RSDesc)
	{
		if (BindDesc.Type == D3D_SIT_SAMPLER)
		{
			return -1;
		}

		// Find correct bind index in RootSignature
		for (uint32 i = 0; i < RSDesc.NumParameters; ++i)
		{
			const D3D12_ROOT_PARAMETER& Parameter = RSDesc.pParameters[i];
			switch (Parameter.ParameterType)
			{
			case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
			{
				const D3D12_ROOT_DESCRIPTOR_TABLE& DescriptorTable = Parameter.DescriptorTable;
				for (uint32 range = 0; range < DescriptorTable.NumDescriptorRanges; ++range)
				{
					const D3D12_DESCRIPTOR_RANGE& DescriptorRange = DescriptorTable.pDescriptorRanges[range];
					if (DescriptorRange.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
					{
						if (BindDesc.Type == D3D_SIT_TEXTURE)
						{
							TI_ASSERT(BindDesc.BindPoint >= DescriptorRange.BaseShaderRegister &&
								BindDesc.BindPoint < DescriptorRange.BaseShaderRegister + DescriptorRange.NumDescriptors);
							return (int32)i;
						}
					}
					else if (DescriptorRange.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_CBV)
					{
						if (BindDesc.Type == D3D_SIT_CBUFFER)
						{
							TI_ASSERT(BindDesc.BindPoint >= DescriptorRange.BaseShaderRegister &&
								BindDesc.BindPoint < DescriptorRange.BaseShaderRegister + DescriptorRange.NumDescriptors);
							return (int32)i;
						}
					}
					else
					{
						// Not support yet.
						TI_ASSERT(0);
					}
				}
			}
				break;
			case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
			case D3D12_ROOT_PARAMETER_TYPE_SRV:
			case D3D12_ROOT_PARAMETER_TYPE_UAV:
			{
				// Not support yet.
				TI_ASSERT(0);
			}
				break;
			case D3D12_ROOT_PARAMETER_TYPE_CBV:
			{
				const D3D12_ROOT_DESCRIPTOR& Descriptor = Parameter.Descriptor;
				if (BindDesc.Type == D3D_SIT_CBUFFER)
				{
					if (BindDesc.BindPoint == Descriptor.ShaderRegister)
					{
						return (int32)i;
					}
				}
			}
				break;
			}
		}

		// Not found correspond param bind index.
		TI_ASSERT(0);
		return -1;
	}

	inline uint32 GetRSDescCrc(const D3D12_ROOT_SIGNATURE_DESC& RSDesc)
	{
		TStream RSData;
		RSData.Put(&RSDesc.NumParameters, sizeof(uint32));
		RSData.Put(&RSDesc.NumStaticSamplers, sizeof(uint32));
		RSData.Put(&RSDesc.Flags, sizeof(uint32));
		for (uint32 i = 0; i < RSDesc.NumParameters; ++i)
		{
			const D3D12_ROOT_PARAMETER& Parameter = RSDesc.pParameters[i];
			RSData.Put(&Parameter.ParameterType, sizeof(uint32));
			switch (Parameter.ParameterType)
			{
			case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
			{
				const D3D12_ROOT_DESCRIPTOR_TABLE& Table = Parameter.DescriptorTable;
				RSData.Put(&Table.NumDescriptorRanges, sizeof(uint32));
				for (uint32 d = 0; d < Table.NumDescriptorRanges; ++d)
				{
					RSData.Put(Table.pDescriptorRanges + d, sizeof(D3D12_DESCRIPTOR_RANGE));
				}
			}
			break;
			case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
			{
				const D3D12_ROOT_CONSTANTS& Constants = Parameter.Constants;
				RSData.Put(&Constants, sizeof(D3D12_ROOT_CONSTANTS));
			}
			break;
			case D3D12_ROOT_PARAMETER_TYPE_CBV:
			case D3D12_ROOT_PARAMETER_TYPE_SRV:
			case D3D12_ROOT_PARAMETER_TYPE_UAV:
			{
				const D3D12_ROOT_DESCRIPTOR& Descriptor = Parameter.Descriptor;
				RSData.Put(&Descriptor, sizeof(D3D12_ROOT_DESCRIPTOR));
			}
			break;
			}
		}
		for (uint32 i = 0; i < RSDesc.NumStaticSamplers; ++i)
		{
			const D3D12_STATIC_SAMPLER_DESC& SamplerDesc = RSDesc.pStaticSamplers[i];
			RSData.Put(&SamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
		}
		return TCrc::MemCrc32(RSData.GetBuffer(), RSData.GetLength());
	}

	bool FRHIDx12::UpdateHardwareResourceShader(FShaderPtr ShaderResource, TShaderPtr InShaderSource)
	{
		// Dx12 shader only need load byte code.
		FShaderDx12 * ShaderDx12 = static_cast<FShaderDx12*>(ShaderResource.get());

		ID3D12RootSignatureDeserializer * RSDeserializer = nullptr;

		if (ShaderResource->GetShaderType() == EST_COMPUTE)
		{
			const TStream& ShaderCode = InShaderSource->GetComputeShaderCode();
			TI_ASSERT(ShaderCode.GetLength() > 0);

			ShaderDx12->ShaderCodes[0] = ShaderCode;
			if (RSDeserializer == nullptr)
			{
				VALIDATE_HRESULT(D3D12CreateRootSignatureDeserializer(ShaderDx12->ShaderCodes[0].GetBuffer(),
					ShaderDx12->ShaderCodes[0].GetLength(),
					__uuidof(ID3D12RootSignatureDeserializer),
					reinterpret_cast<void**>(&RSDeserializer)));
			}
		}
		else
		{
			for (int32 s = 0; s < ESS_COUNT; ++s)
			{
				const TStream& ShaderCode = InShaderSource->GetShaderCode((E_SHADER_STAGE)s);
				//TString ShaderName = ShaderDx12->GetShaderName((E_SHADER_STAGE)s);
				if (ShaderCode.GetLength() > 0)
				{
					ShaderDx12->ShaderCodes[s] = ShaderCode;

					if (RSDeserializer == nullptr)
					{
						VALIDATE_HRESULT(D3D12CreateRootSignatureDeserializer(ShaderDx12->ShaderCodes[s].GetBuffer(),
							ShaderDx12->ShaderCodes[s].GetLength(),
							__uuidof(ID3D12RootSignatureDeserializer),
							reinterpret_cast<void**>(&RSDeserializer)));
					}
				}
			}
		}

		// Create shader binding, also root signature in dx12
		TI_ASSERT(RSDeserializer != nullptr);
		const D3D12_ROOT_SIGNATURE_DESC* RSDesc = RSDeserializer->GetRootSignatureDesc();
		TI_ASSERT(ShaderDx12->ShaderBinding == nullptr);

		// Search for cached shader bindings
		uint32 RSDescKey = GetRSDescCrc(*RSDesc);
		if (ShaderBindingCache.find(RSDescKey) != ShaderBindingCache.end())
		{
			ShaderDx12->ShaderBinding = ShaderBindingCache[RSDescKey];
		}
		else
		{
			ShaderDx12->ShaderBinding = CreateShaderBinding(*RSDesc);
			ShaderBindingCache[RSDescKey] = ShaderDx12->ShaderBinding;

			if (ShaderResource->GetShaderType() == EST_RENDER)
			{
				// Analysis binding argument types
				for (int32 s = 0; s < ESS_COUNT; ++s)
				{
					if (ShaderDx12->ShaderCodes[s].GetLength() > 0)
					{
						THMap<int32, E_ARGUMENT_TYPE> BindingMap;	// Key is Binding Index, Value is ArgumentIndex in Arguments

						ID3D12ShaderReflection* ShaderReflection;
						VALIDATE_HRESULT(D3DReflect(ShaderDx12->ShaderCodes[s].GetBuffer(), ShaderDx12->ShaderCodes[s].GetLength(), IID_PPV_ARGS(&ShaderReflection)));

						D3D12_SHADER_DESC ShaderDesc;
						VALIDATE_HRESULT(ShaderReflection->GetDesc(&ShaderDesc));
						for (uint32 r = 0; r < ShaderDesc.BoundResources; ++r)
						{
							D3D12_SHADER_INPUT_BIND_DESC BindDescriptor;
							VALIDATE_HRESULT(ShaderReflection->GetResourceBindingDesc(r, &BindDescriptor));
							int32 BindIndex = GetBindIndex(BindDescriptor, *RSDesc);
							if (BindIndex >= 0)
							{
								TString BindName = BindDescriptor.Name;
								E_ARGUMENT_TYPE ArgumentType = FShaderBinding::GetArgumentTypeByName(BindName);
								if (BindingMap.find(BindIndex) == BindingMap.end())
								{
									// Not binded, bind it
									ShaderDx12->ShaderBinding->AddShaderArgument(
										(E_SHADER_STAGE)s,
										FShaderBinding::FShaderArgument(BindIndex, ArgumentType));
									BindingMap[BindIndex] = ArgumentType;
								}
								else
								{
									TI_ASSERT(RSDesc->pParameters[BindIndex].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);
									TI_ASSERT(BindingMap[BindIndex] == ArgumentType);
								}
							}
						}
					}
				}
				ShaderDx12->ShaderBinding->PostInitArguments();
			}
		}

		return true;
	}

	FShaderBindingPtr FRHIDx12::CreateShaderBinding(const D3D12_ROOT_SIGNATURE_DESC& RSDesc)
	{
		// Create new shader binding
		FShaderBindingPtr ShaderBinding = ti_new FRootSignatureDx12(RSDesc.NumParameters, RSDesc.NumStaticSamplers);
		FRootSignatureDx12 * RootSignatureDx12 = static_cast<FRootSignatureDx12*>(ShaderBinding.get());

		const int32 NumBindings = RSDesc.NumParameters;
		const int32 NumSamplers = RSDesc.NumStaticSamplers;

		// Init static sampler
		for (int32 i = 0 ; i < NumSamplers ; ++ i)
		{
			const D3D12_STATIC_SAMPLER_DESC& StaticSampler = RSDesc.pStaticSamplers[i];
			RootSignatureDx12->InitStaticSampler(i, StaticSampler);
		}

		// Init shader params
		for (int32 i = 0; i < NumBindings; ++i)
		{
			const D3D12_ROOT_PARAMETER& Parameter = RSDesc.pParameters[i];
			switch (Parameter.ParameterType)
			{
			case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
			{
				const D3D12_ROOT_DESCRIPTOR_TABLE& Table = Parameter.DescriptorTable;
				// Make a copy of Ranges
				D3D12_DESCRIPTOR_RANGE* Ranges = ti_new D3D12_DESCRIPTOR_RANGE[Table.NumDescriptorRanges];
				for (uint32 range = 0; range < Table.NumDescriptorRanges; ++range)
				{
					Ranges[range] = Table.pDescriptorRanges[range];
				}
				RootSignatureDx12->GetParameter(i).InitAsDescriptorTable(Ranges, Table.NumDescriptorRanges, Parameter.ShaderVisibility);
			}
				break;
			case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
			{
				const D3D12_ROOT_CONSTANTS& Constant = Parameter.Constants;
				RootSignatureDx12->GetParameter(i).InitAsConstants(Constant.ShaderRegister, Constant.Num32BitValues, Parameter.ShaderVisibility);
			}
				break;
			case D3D12_ROOT_PARAMETER_TYPE_CBV:
			case D3D12_ROOT_PARAMETER_TYPE_SRV:
			case D3D12_ROOT_PARAMETER_TYPE_UAV:
			{
				const D3D12_ROOT_DESCRIPTOR& Descriptor = Parameter.Descriptor;
				RootSignatureDx12->GetParameter(i).InitAsBuffer(Parameter.ParameterType, Descriptor.ShaderRegister, Parameter.ShaderVisibility);
			}
				break;
			default:
				TI_ASSERT(0);
				break;
			}
		}
		RootSignatureDx12->Finalize(D3dDevice.Get(), RSDesc.Flags);
		HoldResourceReference(ShaderBinding);

		return ShaderBinding;
	}

	// InShader and SpecifiedBindingIndex are used for Metal to create argument buffer, no use in dx12.
	bool FRHIDx12::UpdateHardwareResourceAB(FArgumentBufferPtr ArgumentBuffer, FShaderPtr InShader, int32 SpecifiedBindingIndex)
	{
		FArgumentBufferDx12 * ArgumentDx12 = static_cast<FArgumentBufferDx12*>(ArgumentBuffer.get());
		const TVector<FRenderResourcePtr>& Arguments = ArgumentBuffer->GetArguments();
		TI_ASSERT(ArgumentDx12->ResourceTable == nullptr && Arguments.size() > 0);

		ArgumentDx12->ResourceTable = CreateRenderResourceTable((uint32)Arguments.size(), EHT_SHADER_RESOURCE);
		for (int32 i = 0 ; i < (int32)Arguments.size() ; ++ i)
		{
			FRenderResourcePtr Arg = Arguments[i];
			if (Arg->GetResourceType() == RRT_UNIFORM_BUFFER)
			{
				FUniformBufferPtr ArgUB = static_cast<FUniformBuffer*>(Arg.get());
				ArgumentDx12->ResourceTable->PutUniformBufferInTable(ArgUB, i);
			}
			else if (Arg->GetResourceType() == RRT_TEXTURE)
			{
				FTexturePtr ArgTex = static_cast<FTexture*>(Arg.get());
				ArgumentDx12->ResourceTable->PutTextureInTable(ArgTex, i);
			}
			else
			{
				_LOG(Fatal, "Invalid resource type in Argument buffer.\n");
			}
		}

		return true;
	}

	bool FRHIDx12::UpdateHardwareResourceGPUCommandSig(FGPUCommandSignaturePtr GPUCommandSignature)
	{
		FGPUCommandSignatureDx12 * GPUCommandSignatureDx12 = static_cast<FGPUCommandSignatureDx12*>(GPUCommandSignature.get());

		// Find arguments in command signature
		const TVector<E_GPU_COMMAND_TYPE>& CommandStructure = GPUCommandSignature->GetCommandStructure();

		const uint32 ArgsCount = (uint32)CommandStructure.size();

		// Fill D3D12 INDIRECT ARGUMENT DESC
		D3D12_INDIRECT_ARGUMENT_DESC * ArgumentDescs = ti_new D3D12_INDIRECT_ARGUMENT_DESC[ArgsCount];
		uint32 ArgBytesStride = 0;
		GPUCommandSignatureDx12->ArgumentStrideOffset.resize(CommandStructure.size());
		for (uint32 i = 0 ; i < (uint32)CommandStructure.size() ; ++ i)
		{
			E_GPU_COMMAND_TYPE Command = CommandStructure[i];
			if (Command == GPU_COMMAND_SET_VERTEX_BUFFER)
			{
				// Vertex Buffer
				ArgumentDescs[i].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
				ArgumentDescs[i].VertexBuffer.Slot = 0;	// Vertex Buffer always has Slot 0
			}
			else if (Command == GPU_COMMAND_SET_INSTANCE_BUFFER)
			{
				// Instance Buffer
				ArgumentDescs[i].Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
				ArgumentDescs[i].VertexBuffer.Slot = 1;	// Instance Buffer always has Slot 1
			}
			else if (Command == GPU_COMMAND_SET_INDEX_BUFFER)
			{
				// Index Buffer
				ArgumentDescs[i].Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
			}
			else if (Command == GPU_COMMAND_DRAW_INDEXED)
			{
				ArgumentDescs[i].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
			}
			else
			{
				TI_ASSERT(0);
			}
			GPUCommandSignatureDx12->ArgumentStrideOffset[i] = ArgBytesStride;
			ArgBytesStride += FGPUCommandSignatureDx12::GPU_COMMAND_STRIDE[i];
		}
		GPUCommandSignatureDx12->CommandStrideInBytes = ArgBytesStride;

		D3D12_COMMAND_SIGNATURE_DESC CommandSignatureDesc = {};
		CommandSignatureDesc.pArgumentDescs = ArgumentDescs;
		CommandSignatureDesc.NumArgumentDescs = ArgsCount;
		CommandSignatureDesc.ByteStride = ArgBytesStride;

		TI_TODO("Check if this command signature need render root signature.");
		// The root signature must be specified if and only if the command signature changes one of the root arguments.
		// That's D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT, D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW,
		// D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW, D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW
		//FShaderBindingPtr ShaderBinding = Pipeline->GetShader()->ShaderBinding;
		//FRootSignatureDx12* RenderSignature = static_cast<FRootSignatureDx12*>(ShaderBinding.get());
		//VALIDATE_HRESULT(D3dDevice->CreateCommandSignature(&CommandSignatureDesc, RenderSignature->Get(), IID_PPV_ARGS(&GPUCommandBufferDx12->CommandSignature)));
		VALIDATE_HRESULT(D3dDevice->CreateCommandSignature(&CommandSignatureDesc, nullptr, IID_PPV_ARGS(&GPUCommandSignatureDx12->CommandSignature)));
		ti_delete[] ArgumentDescs;

		HoldResourceReference(GPUCommandSignature);
		return true;
	}

	bool FRHIDx12::UpdateHardwareResourceGPUCommandBuffer(FGPUCommandBufferPtr GPUCommandBuffer)
	{
		FGPUCommandBufferDx12 * GPUCommandBufferDx12 = static_cast<FGPUCommandBufferDx12*>(GPUCommandBuffer.get());
		FGPUCommandSignatureDx12 * SignatueDx12 = static_cast<FGPUCommandSignatureDx12*>(GPUCommandBuffer->GetGPUCommandSignature().get());

		TI_ASSERT(GPUCommandBuffer->GetCommandBuffer() != nullptr);
		UpdateHardwareResourceUB(GPUCommandBuffer->GetCommandBuffer(), GPUCommandBufferDx12->CommandBufferData != nullptr ? GPUCommandBufferDx12->CommandBufferData->GetBuffer() : nullptr);

		HoldResourceReference(GPUCommandBuffer);
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

	void FRHIDx12::PutConstantBufferInHeap(FUniformBufferPtr InUniformBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot)
	{
		FUniformBufferDx12 * UniformBufferDx12 = static_cast<FUniformBufferDx12*>(InUniformBuffer.get());

		const int32 AlignedDataSize = ti_align(InUniformBuffer->GetTotalBufferSize(), UniformBufferAlignSize);
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = UniformBufferDx12->BufferResource.GetResource()->GetGPUVirtualAddress();
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
		D3dDevice->CreateShaderResourceView(TexDx12->TextureResource.GetResource().Get(), &SRVDesc, Descriptor);
	}

	void FRHIDx12::PutUniformBufferInHeap(FUniformBufferPtr InBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot)
	{
		FUniformBufferDx12 * UBDx12 = static_cast<FUniformBufferDx12*>(InBuffer.get());
		
		if ((InBuffer->GetFlag() & UB_FLAG_COMPUTE_WRITABLE) != 0)
		{
			// Create unordered access view
			D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
			UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
			UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			UAVDesc.Buffer.FirstElement = 0;
			UAVDesc.Buffer.NumElements = InBuffer->GetElements();
			UAVDesc.Buffer.StructureByteStride = InBuffer->GetStructureSizeInBytes();
			UAVDesc.Buffer.CounterOffsetInBytes = (InBuffer->GetFlag() & UB_FLAG_COMPUTE_WITH_COUNTER) != 0 ? 
				FUniformBufferDx12::AlignForUavCounter(InBuffer->GetElements() * InBuffer->GetStructureSizeInBytes()) : 0;
			UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

			D3D12_CPU_DESCRIPTOR_HANDLE Descriptor = GetCpuDescriptorHandle(InHeapType, InHeapSlot);

			D3dDevice->CreateUnorderedAccessView(
				UBDx12->BufferResource.GetResource().Get(),
				(InBuffer->GetFlag() & UB_FLAG_COMPUTE_WITH_COUNTER) != 0 ?
					UBDx12->BufferResource.GetResource().Get() : nullptr,
				&UAVDesc,
				Descriptor);
		}
		else
		{
			// Create shader resource view
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
			SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			SRVDesc.Buffer.NumElements = InBuffer->GetElements();
			SRVDesc.Buffer.StructureByteStride = InBuffer->GetStructureSizeInBytes();
			SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			D3D12_CPU_DESCRIPTOR_HANDLE Descriptor = GetCpuDescriptorHandle(InHeapType, InHeapSlot);
			D3dDevice->CreateShaderResourceView(UBDx12->BufferResource.GetResource().Get(), &SRVDesc, Descriptor);
		}
	}

	void FRHIDx12::PutInstanceBufferInHeap(FInstanceBufferPtr InInstanceBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot)
	{
		TI_TODO("Refactor this with PutUniformBufferInHeap().");
		FInstanceBufferDx12 * IBDx12 = static_cast<FInstanceBufferDx12*>(InInstanceBuffer.get());

		// Create shader resource view
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		SRVDesc.Buffer.NumElements = InInstanceBuffer->GetInstancesCount();
		SRVDesc.Buffer.StructureByteStride = InInstanceBuffer->GetStride();
		SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		D3D12_CPU_DESCRIPTOR_HANDLE Descriptor = GetCpuDescriptorHandle(InHeapType, InHeapSlot);
		D3dDevice->CreateShaderResourceView(IBDx12->InstanceBuffer.Get(), &SRVDesc, Descriptor);
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
		D3dDevice->CreateRenderTargetView(TexDx12->TextureResource.GetResource().Get(), &RTVDesc, Descriptor);
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
		D3dDevice->CreateDepthStencilView(TexDx12->TextureResource.GetResource().Get(), &DsvDesc, Descriptor);
	}

	void FRHIDx12::SetGraphicsPipeline(FPipelinePtr InPipeline)
	{
		if (CurrentBoundResource.Pipeline != InPipeline)
		{
			FPipelineDx12* PipelineDx12 = static_cast<FPipelineDx12*>(InPipeline.get());
			FShaderPtr Shader = InPipeline->GetShader();
			TI_ASSERT(Shader->GetShaderType() == EST_RENDER);
			FShaderBindingPtr ShaderBinding = Shader->ShaderBinding;
			TI_ASSERT(ShaderBinding != nullptr);

			if (CurrentBoundResource.ShaderBinding != ShaderBinding)
			{
				FRootSignatureDx12 * RSDx12 = static_cast<FRootSignatureDx12*>(ShaderBinding.get());
				CurrentWorkingCommandList->SetGraphicsRootSignature(RSDx12->Get());
				CurrentBoundResource.ShaderBinding = ShaderBinding;
			}

			CurrentWorkingCommandList->SetPipelineState(PipelineDx12->PipelineState.Get());

			HoldResourceReference(InPipeline);

			CurrentBoundResource.Pipeline = InPipeline;
		}
	}

	void FRHIDx12::SetComputePipeline(FPipelinePtr InPipeline)
	{
		FPipelineDx12* PipelineDx12 = static_cast<FPipelineDx12*>(InPipeline.get());
		FShaderPtr Shader = InPipeline->GetShader();
		TI_ASSERT(Shader->GetShaderType() == EST_COMPUTE);
		FShaderBindingPtr ShaderBinding = Shader->ShaderBinding;
		TI_ASSERT(ShaderBinding != nullptr);

		CurrentWorkingCommandList->SetPipelineState(PipelineDx12->PipelineState.Get());

		FRootSignatureDx12 * RSDx12 = static_cast<FRootSignatureDx12*>(ShaderBinding.get());
		CurrentWorkingCommandList->SetComputeRootSignature(RSDx12->Get());

		HoldResourceReference(InPipeline);
	}

	void FRHIDx12::SetMeshBuffer(FMeshBufferPtr InMeshBuffer, FInstanceBufferPtr InInstanceBuffer)
	{
		FMeshBufferDx12* MBDx12 = static_cast<FMeshBufferDx12*>(InMeshBuffer.get());

		TI_TODO("Remove duplicated IASetPrimitiveTopology and IASetVertexBuffers call similar as SetGraphicsPipeline()");
		CurrentWorkingCommandList->IASetPrimitiveTopology(k_PRIMITIVE_TYPE_MAP[InMeshBuffer->GetPrimitiveType()]);
		if (InInstanceBuffer == nullptr)
		{
			CurrentWorkingCommandList->IASetVertexBuffers(0, 1, &MBDx12->VertexBufferView);
		}
		else
		{
			FInstanceBufferDx12* IBDx12 = static_cast<FInstanceBufferDx12*>(InInstanceBuffer.get());
			D3D12_VERTEX_BUFFER_VIEW Views[2] =
			{
				MBDx12->VertexBufferView,
				IBDx12->InstanceBufferView
			};
			CurrentWorkingCommandList->IASetVertexBuffers(0, 2, Views);
			HoldResourceReference(InInstanceBuffer);
		}
		CurrentWorkingCommandList->IASetIndexBuffer(&MBDx12->IndexBufferView);

		HoldResourceReference(InMeshBuffer);
	}

	void FRHIDx12::SetMeshBufferAtSlot(uint32 StartSlot, FMeshBufferPtr InMeshBuffer)
	{
		TI_ASSERT(0);
	}

	void FRHIDx12::SetInstanceBufferAtSlot(uint32 StartSlot, FInstanceBufferPtr InInstanceBuffer)
	{
		FInstanceBufferDx12* IBDx12 = static_cast<FInstanceBufferDx12*>(InInstanceBuffer.get());

		CurrentWorkingCommandList->IASetVertexBuffers(StartSlot, 1, &IBDx12->InstanceBufferView);
	}

	void FRHIDx12::SetUniformBuffer(E_SHADER_STAGE , int32 BindIndex, FUniformBufferPtr InUniformBuffer)
	{
		FUniformBufferDx12* UBDx12 = static_cast<FUniformBufferDx12*>(InUniformBuffer.get());

		// Bind the current frame's constant buffer to the pipeline.
		CurrentWorkingCommandList->SetGraphicsRootConstantBufferView(BindIndex, UBDx12->BufferResource.GetResource()->GetGPUVirtualAddress());

		HoldResourceReference(InUniformBuffer);
	}

	void FRHIDx12::SetComputeBuffer(int32 BindIndex, FUniformBufferPtr InUniformBuffer)
	{
		FUniformBufferDx12* UBDx12 = static_cast<FUniformBufferDx12*>(InUniformBuffer.get());

		// Bind the current frame's constant buffer to the pipeline.
		CurrentWorkingCommandList->SetComputeRootConstantBufferView(BindIndex, UBDx12->BufferResource.GetResource()->GetGPUVirtualAddress());

		HoldResourceReference(InUniformBuffer);
	}

	void FRHIDx12::SetRenderResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE Descriptor = GetGpuDescriptorHandle(RenderResourceTable->GetHeapType(), RenderResourceTable->GetStartIndex());
		CurrentWorkingCommandList->SetGraphicsRootDescriptorTable(BindIndex, Descriptor);

		HoldResourceReference(RenderResourceTable);
	}

	void FRHIDx12::SetComputeResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE Descriptor = GetGpuDescriptorHandle(RenderResourceTable->GetHeapType(), RenderResourceTable->GetStartIndex());
		CurrentWorkingCommandList->SetComputeRootDescriptorTable(BindIndex, Descriptor);

		HoldResourceReference(RenderResourceTable);
	}

	void FRHIDx12::SetComputeArgumentBuffer(int32 InBindIndex, FArgumentBufferPtr InArgumentBuffer)
	{
		FArgumentBufferDx12 * ArgDx12 = static_cast<FArgumentBufferDx12*>(InArgumentBuffer.get());
		SetComputeResourceTable(InBindIndex, ArgDx12->ResourceTable);
	}
	
	void FRHIDx12::SetComputeTexture(int32 BindIndex, FTexturePtr InTexture)
	{
		TI_ASSERT(0);
	}

	void FRHIDx12::DispatchCompute(const vector3di& GroupSize, const vector3di& GroupCount)
	{
		TI_ASSERT(CurrentCommandListState.ListType == EPL_COMPUTE);
		CurrentWorkingCommandList->Dispatch(GroupCount.X, GroupCount.Y, GroupCount.Z);
	}

	void FRHIDx12::ComputeCopyBuffer(FUniformBufferPtr Dest, uint32 DestOffset, FUniformBufferPtr Src, uint32 SrcOffset, uint32 CopySize)
	{
		FUniformBufferDx12 * DestDx12 = static_cast<FUniformBufferDx12*>(Dest.get());
		FUniformBufferDx12 * SrcDx12 = static_cast<FUniformBufferDx12*>(Src.get());

		CurrentWorkingCommandList->CopyBufferRegion(
			DestDx12->BufferResource.GetResource().Get(), DestOffset,
			SrcDx12->BufferResource.GetResource().Get(), SrcOffset,
			CopySize);
	}

	void FRHIDx12::ExecuteGPUCommands(FGPUCommandBufferPtr GPUCommandBuffer)
	{
		if (GPUCommandBuffer->GetEncodedCommandsCount() > 0)
		{
			FGPUCommandBufferDx12 * GPUCommandBufferDx12 = static_cast<FGPUCommandBufferDx12*>(GPUCommandBuffer.get());
			FGPUCommandSignatureDx12 * SignatueDx12 = static_cast<FGPUCommandSignatureDx12*>(GPUCommandBuffer->GetGPUCommandSignature().get());

			// Set pipeline state
			SetGraphicsPipeline(SignatueDx12->GetPipeline());

			// Set public binding arguments
			const TVector<FUniformBufferPtr>& VSArguments = GPUCommandBuffer->GetVSPublicArguments();
			const TVector<FUniformBufferPtr>& PSArguments = GPUCommandBuffer->GetPSPublicArguments();
			for (int32 i = 0 ; i < FGPUCommandBuffer::MAX_BINDING_UNIFORMS ; ++ i)
			{
				if (VSArguments[i] != nullptr)
				{
					SetUniformBuffer(ESS_VERTEX_SHADER, i, VSArguments[i]);
				}
			}
			for (int32 i = 0; i < FGPUCommandBuffer::MAX_BINDING_UNIFORMS; ++i)
			{
				if (PSArguments[i] != nullptr)
				{
					SetUniformBuffer(ESS_PIXEL_SHADER, i, PSArguments[i]);
				}
			}

			// Set Primitive Topology
			CurrentWorkingCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			
			// Execute indirect draw.
			FUniformBufferDx12 * CommandBuffer = static_cast<FUniformBufferDx12*>(GPUCommandBuffer->GetCommandBuffer().get());
			if ((CommandBuffer->GetFlag() & UB_FLAG_COMPUTE_WITH_COUNTER) != 0)
			{
				uint32 CounterOffset = CommandBuffer->GetCounterOffset();
				CurrentWorkingCommandList->ExecuteIndirect(
					SignatueDx12->CommandSignature.Get(),
					GPUCommandBuffer->GetEncodedCommandsCount(),
					CommandBuffer->BufferResource.GetResource().Get(),
					0,
					CommandBuffer->BufferResource.GetResource().Get(),
					CounterOffset);
			}
			else
			{
				CurrentWorkingCommandList->ExecuteIndirect(
					SignatueDx12->CommandSignature.Get(),
					GPUCommandBuffer->GetEncodedCommandsCount(),
					CommandBuffer->BufferResource.GetResource().Get(),
					0,
					nullptr,
					0);
			}

			HoldResourceReference(GPUCommandBuffer);
		}
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

	void FRHIDx12::SetArgumentBuffer(int32 InBindIndex, FArgumentBufferPtr InArgumentBuffer)
	{
		TI_ASSERT(InBindIndex >= 0);
		FArgumentBufferDx12 * ArgDx12 = static_cast<FArgumentBufferDx12*>(InArgumentBuffer.get());
		SetRenderResourceTable(InBindIndex, ArgDx12->ResourceTable);
	}

	void FRHIDx12::SetStencilRef(uint32 InRefValue)
	{
		CurrentWorkingCommandList->OMSetStencilRef(InRefValue);
	}

	void FRHIDx12::DrawPrimitiveIndexedInstanced(FMeshBufferPtr MeshBuffer, uint32 InstanceCount, uint32 InstanceOffset)
	{
		TI_ASSERT(CurrentCommandListState.ListType == EPL_GRAPHICS);
		CurrentWorkingCommandList->DrawIndexedInstanced(MeshBuffer->GetIndicesCount(), InstanceCount, 0, 0, InstanceOffset);

		FStats::Stats.TrianglesRendered += MeshBuffer->GetIndicesCount() / 3 * InstanceCount;
	}

	void FRHIDx12::GraphicsCopyBuffer(FUniformBufferPtr Dest, uint32 DestOffset, FUniformBufferPtr Src, uint32 SrcOffset, uint32 CopySize)
	{
		TI_ASSERT(0);
	}

	void FRHIDx12::SetTilePipeline(FPipelinePtr InPipeline)
	{
		TI_ASSERT(0);
	}

	void FRHIDx12::SetTileBuffer(int32 BindIndex, FUniformBufferPtr InUniformBuffer)
	{
		TI_ASSERT(0);
	}

	void FRHIDx12::DispatchTile(const vector3di& GroupSize)
	{
		TI_ASSERT(0);
	}

	void FRHIDx12::SetViewport(const FViewport& VP)
	{
		FRHI::SetViewport(VP);

		D3D12_VIEWPORT ViewportDx = { float(VP.Left), float(VP.Top), float(VP.Width), float(VP.Height), 0.f, 1.f };
		CurrentWorkingCommandList->RSSetViewports(1, &ViewportDx);
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
		FlushGraphicsBarriers(CurrentWorkingCommandList.Get());

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
		CurrentWorkingCommandList->OMSetRenderTargets(RT->GetColorBufferCount(), Rtv, false, Dsv);

		// Clear render target
		if (CBCount > 0)
		{
			for (int32 cb = 0; cb < CBCount; ++cb)
			{
				CurrentWorkingCommandList->ClearRenderTargetView(RTVDescriptors[cb], DirectX::Colors::Transparent, 0, nullptr);
			}
		}
		if (Dsv != nullptr)
		{
			CurrentWorkingCommandList->ClearDepthStencilView(*Dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		}
	}

	void FRHIDx12::BeginRenderToRenderTarget(FRenderTargetPtr RT, const int8* PassName)
	{
		FRHI::BeginRenderToRenderTarget(RT, PassName);

		SetRenderTarget(RT);
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