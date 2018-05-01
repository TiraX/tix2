/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"

#if COMPILE_WITH_RHI_DX12
#include "TDeviceWin32.h"

// link libraries
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")

namespace tix
{
#define VALIDATE_HRESULT(hr) TI_ASSERT(SUCCEEDED(hr))
	FRHIDx12::FRHIDx12()
		: CurrentFrame(0)
	{
		Init();
	}

	FRHIDx12::~FRHIDx12()
	{
	}

	void FRHIDx12::Init()
	{
		// Create D3D12 Device
		HRESULT hr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&DxgiFactory));

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

		// Create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		hr = D3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue));
		VALIDATE_HRESULT(hr);
		CommandQueue->SetName(L"CommandQueue");

		// Create descriptor heaps for render target views and depth stencil views.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FRHI::FrameBufferNum;
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

		for (uint32 n = 0; n < FRHI::FrameBufferNum; n++)
		{
			hr = D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocators[n]));
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
		_LOG("  RHI DirectX 12 inited.\n");
	}

	// This method acquires the first available hardware adapter that supports Direct3D 12.
	// If no such adapter can be found, *ppAdapter will be set to nullptr.
	void FRHIDx12::GetHardwareAdapter(IDXGIAdapter1** ppAdapter)
	{
		ComPtr<IDXGIAdapter1> adapter;
		*ppAdapter = nullptr;

		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != DxgiFactory->EnumAdapters1(adapterIndex, &adapter); adapterIndex++)
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
				break;
			}
		}

		*ppAdapter = adapter.Detach();
	}

	void FRHIDx12::CreateWindowsSizeDependentResources()
	{
		// Clear the previous window size specific content and update the tracked fence values.
		for (uint32 n = 0; n < FRHI::FrameBufferNum; n++)
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

		DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
		DXGI_FORMAT DepthBufferFormat = DXGI_FORMAT_D32_FLOAT;

		if (SwapChain != nullptr)
		{
			// If the swap chain already exists, resize it.
			HRESULT hr = SwapChain->ResizeBuffers(FRHI::FrameBufferNum, BackBufferWidth, BackBufferHeight, BackBufferFormat, 0);

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
			swapChainDesc.BufferCount = FRHI::FrameBufferNum;			// Use triple-buffering to minimize latency.
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// All Windows Universal apps must use _FLIP_ SwapEffects.
			swapChainDesc.Flags = 0;
			swapChainDesc.Scaling = DXGI_SCALING_NONE;
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
			for (UINT n = 0; n < FRHI::FrameBufferNum; n++)
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

		// Set the 3D rendering viewport to target the entire window.
		//m_screenViewport = { 0.0f, 0.0f, m_d3dRenderTargetSize.Width, m_d3dRenderTargetSize.Height, 0.0f, 1.0f };

	}

	void FRHIDx12::ClearBuffers()
	{
	}
}
#endif	// COMPILE_WITH_RHI_DX12