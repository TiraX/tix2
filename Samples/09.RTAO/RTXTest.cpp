/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "RTXTest.h"

// link libraries
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "d3dcompiler.lib")

const int32 WinWidth = 1600;
const int32 WinHeight = 900;

const DXGI_FORMAT OutputTextureFormat = DXGI_FORMAT_B8G8R8A8_UNORM;// DXGI_FORMAT_R8G8B8A8_UNORM;

//------------------------------------------------------------------------------------------------
// All arrays must be populated (e.g. by calling GetCopyableFootprints)
uint64 UpdateSubresources(
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
uint64 UpdateSubresources(
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

TRTXTest::TRTXTest()
	: Inited(false)
	, HWnd(0)
	, CurrentFrame(0)
	, MBVertexCount(0)
	, MBVertexStride(0)
	, MBIndexCount(0)
	, DispatchResourceTableStart(0)
	, ConstantBufferMappedAddress(nullptr)
{
	// Get app current work dir.
	int8 szFilePath[MAX_PATH + 1] = { 0 };
	GetCurrentDirectory(MAX_PATH, szFilePath);
	TString WorkPath = szFilePath;
	// Change work to Cooked directory.
	TString AbsolutePath = WorkPath + "\\Cooked\\Windows\\";
	SetCurrentDirectory(AbsolutePath.c_str());
}

TRTXTest::~TRTXTest()
{
}

void TRTXTest::Run()
{
	if (!Inited)
	{
		Init();
		Inited = true;
	}

	while (IsRunning())
	{
		Tick();
		Render();
	}
}

void TRTXTest::Init()
{
	CreateAWindow();
	InitD3D12();
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

	static int ClickCount = 0;
	if (GetCapture() != hWnd && ClickCount > 0)
		ClickCount = 0;

	int x, y;
	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	return 0;

	case WM_ERASEBKGND:
		return 0;

	case WM_SETCURSOR:
		break;

	case WM_MOUSEWHEEL:
	{
	}
	break;

	case WM_RBUTTONDOWN:
		ClickCount++;
		SetCapture(hWnd);
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		return 0;

	case WM_RBUTTONUP:
		ClickCount--;
		if (ClickCount < 1)
		{
			ClickCount = 0;
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			ReleaseCapture();
		}
		return 0;

	case WM_LBUTTONDOWN:
		ClickCount++;
		SetCapture(hWnd);
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		return 0;

	case WM_LBUTTONUP:
		ClickCount--;
		if (ClickCount < 1)
		{
			ClickCount = 0;
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			ReleaseCapture();
		}
		return 0;

	case WM_MBUTTONDOWN:
		ClickCount++;
		SetCapture(hWnd);
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		return 0;

	case WM_MBUTTONUP:
		ClickCount--;
		if (ClickCount < 1)
		{
			ClickCount = 0;
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			ReleaseCapture();
		}
		return 0;

	case WM_MOUSEMOVE:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		return 0;

	case WM_KEYDOWN:
		return 0;
	case WM_KEYUP:
		return 0;

	case WM_SIZE:
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_SCREENSAVE ||
			(wParam & 0xFFF0) == SC_MONITORPOWER)
			return 0;
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void TRTXTest::CreateAWindow()
{
	HINSTANCE hInstance = GetModuleHandle(0);
	const char* windowName = "RTXTest App";
	// create the window, only if we do not use the null device
	if (HWnd == 0)
	{
		// Register Class
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = (WNDPROC)WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = 0;
		wcex.lpszClassName = windowName;
		wcex.hIconSm = 0;

		RegisterClassEx(&wcex);

		// calculate client size

		RECT clientSize;
		clientSize.top = 0;
		clientSize.left = 0;
		clientSize.right = WinWidth;
		clientSize.bottom = WinHeight;

		DWORD style = WS_POPUP;

		style = WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

		AdjustWindowRect(&clientSize, style, FALSE);

		int realWidth = clientSize.right - clientSize.left;
		int realHeight = clientSize.bottom - clientSize.top;

		int windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
		int windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

		// create window
		HWnd = CreateWindow(windowName, windowName, style, windowLeft, windowTop,
			realWidth, realHeight, NULL, NULL, hInstance, NULL);
		TI_ASSERT(HWnd);
		if (!HWnd)
		{
			_LOG(Fatal, "Create window failed.\n");
			return;
		}

		ShowWindow(HWnd, SW_SHOW);
		UpdateWindow(HWnd);

		MoveWindow(HWnd, windowLeft, windowTop, realWidth, realHeight, TRUE);
	}
	else
	{
		RECT r;
		GetWindowRect(HWnd, &r);
		//Width = r.right - r.left;
		//Height = r.bottom - r.top;
	}

	// set this as active window
	SetActiveWindow(HWnd);
	SetForegroundWindow(HWnd);
}

bool TRTXTest::IsRunning()
{
	MSG msg;

	bool quit = false;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);

		if (msg.hwnd == HWnd)
			WndProc(HWnd, msg.message, msg.wParam, msg.lParam);
		else
			DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			quit = true;
	}

	return !quit;
}

void TRTXTest::InitD3D12()
{
	HRESULT Hr;

#if defined(TIX_DEBUG)
	// If the project is in a debug build, enable debugging via SDK Layers.
	{
		ComPtr<ID3D12Debug> DebugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
		{
			// Disable Debug layer for Nsights Graphics to profile shader and GPU trace.
			DebugController->EnableDebugLayer();
		}
		else
		{
			_LOG(Warning, "Direct3D Debug Device is NOT avaible.\n");
		}

		// Try to create debug factory
		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
		{
			VALIDATE_HRESULT(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&DxgiFactory)));

			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
		}
		else
		{
			// Failed to create debug factory, create a normal one
			VALIDATE_HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&DxgiFactory)));
		}
	}
#else
	// Create D3D12 Device
	VALIDATE_HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&DxgiFactory)));
#endif

	ComPtr<IDXGIAdapter1> Adapter;
	ComPtr<IDXGIFactory6> Factory6;

	Hr = DxgiFactory.As(&Factory6);
	if (FAILED(Hr))
	{
		// Get Adapter use a safe way
		for (uint32 AdapterIndex = 0; DXGI_ERROR_NOT_FOUND != DxgiFactory->EnumAdapters1(AdapterIndex, &Adapter); AdapterIndex++)
		{
			DXGI_ADAPTER_DESC1 desc;
			Adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				continue;
			}

			// Check to see if the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				char AdapterName[128];
				size_t Converted;
				wcstombs_s(&Converted, AdapterName, 128, desc.Description, 128);
				_LOG(Log, "D3D12-capable hardware found:  %s (%u MB)\n", AdapterName, desc.DedicatedVideoMemory >> 20);
				break;
			}
		}
	}
	else
	{
		for (uint32 AdapterIndex = 0; DXGI_ERROR_NOT_FOUND != Factory6->EnumAdapterByGpuPreference(AdapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&Adapter)); AdapterIndex++)
		{
			DXGI_ADAPTER_DESC1 Desc;
			Adapter->GetDesc1(&Desc);

			if (Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				continue;
			}

			// Check to see if the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				char AdapterName[128];
				size_t Converted;
				wcstombs_s(&Converted, AdapterName, 128, Desc.Description, 128);
				_LOG(Log, "D3D12-capable hardware found:  %s (%u MB)\n", AdapterName, Desc.DedicatedVideoMemory >> 20);
				break;
			}
		}

	}

	// Create the Direct3D 12 API device object
	Hr = D3D12CreateDevice(
		Adapter.Get(),					// The hardware adapter.
		D3D_FEATURE_LEVEL_11_0,			// Minimum feature level this app can support.
		IID_PPV_ARGS(&D3dDevice)		// Returns the Direct3D device created.
	);

#if defined(TIX_DEBUG)
	// Do NOT create WARP device
	// WARP is a high speed, fully conformant software rasterizer.
	//if (FAILED(Hr))
	//{
	//	// If the initialization fails, fall back to the WARP device.
	//	// For more information on WARP, see: 
	//	// https://go.microsoft.com/fwlink/?LinkId=286690

	//	ComPtr<IDXGIAdapter> warpAdapter;
	//	DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

	//	Hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&D3dDevice));
	//}
#endif
	VALIDATE_HRESULT(Hr);
#if defined(TIX_DEBUG)
	// Configure debug device (if active).
	ComPtr<ID3D12InfoQueue> D3dInfoQueue;
	if (SUCCEEDED(D3dDevice.As(&D3dInfoQueue)))
	{
		D3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		D3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		D3D12_MESSAGE_ID hide[] =
		{
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
		};
		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = _countof(hide);
		filter.DenyList.pIDList = hide;
		D3dInfoQueue->AddStorageFilterEntries(&filter);
	}
#endif

	// Prevent the GPU from over-clocking or under-clocking to get consistent timings
	//if (DeveloperModeEnabled)
	//	D3dDevice->SetStablePowerState(TRUE);

	// Create the command queue. Graphics and Compute
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	VALIDATE_HRESULT(D3dDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&GraphicsCommandQueue)));
	GraphicsCommandQueue->SetName(L"GraphicsCommandQueue");

	// Create descriptor heaps for render target views and depth stencil views.

	static const int32 MaxDescriptorCount[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
	{
		MAX_HEAP_SRV_CBV,	//EHT_CBV_SRV_UAV,
		MAX_HEAP_SAMPLERS,	//EHT_SAMPLER,
		MAX_HEAP_RENDERTARGETS,	//EHT_RTV,
		MAX_HEAP_DEPTHSTENCILS,	//EHT_DSV,
	};
	static const D3D12_DESCRIPTOR_HEAP_FLAGS k_HEAP_CREATE_FLAGS[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
	{
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,	//EHT_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,	//EHT_SAMPLER,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,	//EHT_RTV,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,	//EHT_DSV,
	};
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
	HeapDesc.NumDescriptors = MaxDescriptorCount[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	HeapDesc.Flags = k_HEAP_CREATE_FLAGS[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
	HeapDesc.NodeMask = 0;
	VALIDATE_HRESULT(D3dDevice->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_RTV])));
	DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_RTV]->SetName(L"HeapRTV");
	DescriptorIncSize[D3D12_DESCRIPTOR_HEAP_TYPE_RTV] = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//RHIDx12->InitRHIRenderResourceHeap(GetTiXHeapTypeFromDxHeap(InHeapType), MaxDescriptorCount[InHeapType], 0);


	HeapDesc.NumDescriptors = MaxDescriptorCount[D3D12_DESCRIPTOR_HEAP_TYPE_DSV];
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	HeapDesc.Flags = k_HEAP_CREATE_FLAGS[D3D12_DESCRIPTOR_HEAP_TYPE_DSV];
	HeapDesc.NodeMask = 0;
	VALIDATE_HRESULT(D3dDevice->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_DSV])));
	DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_DSV]->SetName(L"HeapDSV");
	DescriptorIncSize[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	FenceValues[CurrentFrame]++;

	FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (FenceEvent == nullptr)
	{
		Hr = HRESULT_FROM_WIN32(GetLastError());
		TI_ASSERT(0);
	}

	// CreateWindowsSizeDependentResources();

		// Clear the previous window size specific content and update the tracked fence values.
	for (uint32 n = 0; n < FRHIConfig::FrameBufferNum; n++)
	{
		BackBufferRTs[n] = nullptr;
		FenceValues[n] = FenceValues[CurrentFrame];
	}

	// The width and height of the swap chain must be based on the window's
	// natively-oriented width and height. If the window is not in the native
	// orientation, the dimensions must be reversed.
	DXGI_MODE_ROTATION displayRotation = DXGI_MODE_ROTATION_IDENTITY;

	uint32 BackBufferWidth = lround(WinWidth);
	uint32 BackBufferHeight = lround(WinHeight);

	HRESULT hr;

	const DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	const DXGI_FORMAT DepthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

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
		for (uint32 n = 0; n < FRHIConfig::FrameBufferNum; n++)
		{
			BackBufferDescriptors[n].ptr = DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_RTV]->GetCPUDescriptorHandleForHeapStart().ptr + n * DescriptorIncSize[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
			VALIDATE_HRESULT(SwapChain->GetBuffer(n, IID_PPV_ARGS(&BackBufferRTs[n])));
			D3dDevice->CreateRenderTargetView(BackBufferRTs[n].Get(), nullptr, BackBufferDescriptors[n]);

			WCHAR name[32];
			if (swprintf_s(name, L"BackBufferRTs[%u]", n) > 0)
			{
				BackBufferRTs[n].Get()->SetName(name);
			}
		}
		CurrentFrame = SwapChain->GetCurrentBackBufferIndex();
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

		DepthStencilDescriptor.ptr = DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_DSV]->GetCPUDescriptorHandleForHeapStart().ptr;
		D3dDevice->CreateDepthStencilView(DepthStencil.Get(), &dsvDesc, DepthStencilDescriptor);
	}


	// Describe and create a shader resource view (SRV) heap for the texture.
	HeapDesc.NumDescriptors = MaxDescriptorCount[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	HeapDesc.Flags = k_HEAP_CREATE_FLAGS[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
	HeapDesc.NodeMask = 0;
	VALIDATE_HRESULT(D3dDevice->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV])));
	DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->SetName(L"HeapCBV_SRV_UAV");
	DescriptorIncSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Pre-allocate memory to avoid alloc in runtime
	// Create default command list
	for (uint32 n = 0; n < FRHIConfig::FrameBufferNum; n++)
	{
		VALIDATE_HRESULT(D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&GraphicsAllocators[n])));
		GraphicsAllocators[n]->SetName(L"GraphicsAllocator");
	}
	VALIDATE_HRESULT(D3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		GraphicsAllocators[0].Get(),
		nullptr,
		IID_PPV_ARGS(&GraphicsCommandList)));
	GraphicsCommandList->SetName(L"GraphicsCommandList");
	VALIDATE_HRESULT(GraphicsCommandList->Close());
	VALIDATE_HRESULT(D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&GraphicsFence)));
	GraphicsFence->SetName(L"GraphicsFence");

	// Init raytracing

	VALIDATE_HRESULT(D3dDevice->QueryInterface(IID_PPV_ARGS(&DXRDevice)));
	VALIDATE_HRESULT(GraphicsCommandList->QueryInterface(IID_PPV_ARGS(&DXRCommandList)));
}

void TRTXTest::CreateResources()
{
	InitCamera();
	CreateOutputTexture();
	CreateRaytracingPipelineObject();
	LoadMeshBuffer();
	BuildAccelerationStructures();
	BuildShaderTables();
	CreateShaderParameters();
}

const vector3df CamPos = vector3df(-2.12574f, -1.05096f, 1.22416f);
const vector3df CamTar = vector3df(-4.25856f, 8.56625f, -0.49643f);

void TRTXTest::InitCamera()
{
	MatProj = buildProjectionMatrixPerspectiveFov(0.88357f, 1.7778f, 1.f, 3000.f);

	vector3df CamDir = CamTar - CamPos;
	CamDir.normalize();

	vector3df up = vector3df(0 ,0, 1);
	float32 dp = CamDir.dotProduct(up);

	if (TMath::Equals(fabs(dp), 1.f))
	{
		up.setX(up.getX() + 0.5f);
	}

	MatView = buildCameraLookAtMatrix(CamPos, CamTar, up);
}

void TRTXTest::CreateOutputTexture()
{
	D3D12_CLEAR_VALUE ClearValue = {};
	D3D12_RESOURCE_DESC TextureDx12Desc = {};
	TextureDx12Desc.Alignment = 0;
	TextureDx12Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	TextureDx12Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	TextureDx12Desc.Format = OutputTextureFormat;
	TextureDx12Desc.Width = WinWidth;
	TextureDx12Desc.Height = WinHeight;
	TextureDx12Desc.DepthOrArraySize = 1;
	TextureDx12Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	TextureDx12Desc.MipLevels = 1;
	TextureDx12Desc.SampleDesc.Count = 1;
	TextureDx12Desc.SampleDesc.Quality = 0;

	ClearValue.Color[0] = 0.f;
	ClearValue.Color[1] = 1.f;
	ClearValue.Color[2] = 0.f;
	ClearValue.Color[3] = 1.f;
	ClearValue.Format = OutputTextureFormat;

	CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
		&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&TextureDx12Desc,
		D3D12_RESOURCE_STATE_COMMON,
		&ClearValue,
		IID_PPV_ARGS(&OutputTexture)));
	OutputTextureState = D3D12_RESOURCE_STATE_COMMON;
	OutputTexture->SetName(L"OutputTexture");
}

void TRTXTest::CreateShaderParameters()
{
	// Create constant buffer
	TI_ASSERT(ConstantBuffer == nullptr);
	const int32 CBSize = sizeof(float) * 4 + sizeof(float) * 16;
	CD3DX12_RESOURCE_DESC ConstantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(CBSize);
	CD3DX12_HEAP_PROPERTIES UploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	VALIDATE_HRESULT(DXRDevice->CreateCommittedResource(
		&UploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ConstantBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&ConstantBuffer)));
	ConstantBuffer->SetName(L"ConstantBuffer");

	// Calc constant buffer data
	matrix4 VP = MatProj * MatView;
	matrix4 InvVP;
	VP.getInverse(InvVP);

	FFloat4 PosData = FFloat4(CamPos.X, CamPos.Y, CamPos.Z, 1.f);
	FMatrix ProjectionToWorld = InvVP;

	// Map the constant buffers.
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	VALIDATE_HRESULT(ConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&ConstantBufferMappedAddress)));
	memcpy(ConstantBufferMappedAddress, &PosData, sizeof(FFloat4));
	memcpy(ConstantBufferMappedAddress + sizeof(FFloat4), &ProjectionToWorld, sizeof(FMatrix));
	ConstantBuffer->Unmap(0, nullptr);

	// Put Outputtexture render table
	DispatchResourceTableStart = 8;
	D3D12_CPU_DESCRIPTOR_HANDLE HeapStart;
	D3D12_CPU_DESCRIPTOR_HANDLE ResourceTable;
	HeapStart = DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetCPUDescriptorHandleForHeapStart();
	ResourceTable.ptr = HeapStart.ptr + (DispatchResourceTableStart + INDEX_OUTPUT_TEXTURE) * DescriptorIncSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];

	D3D12_SHADER_RESOURCE_VIEW_DESC OutputTextureSRVDesc = {};
	OutputTextureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	OutputTextureSRVDesc.Format = OutputTextureFormat;
	OutputTextureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	OutputTextureSRVDesc.Texture2D.MipLevels = 1;
	D3dDevice->CreateShaderResourceView(OutputTexture.Get(), &OutputTextureSRVDesc, ResourceTable);

	// Put Acceleration structure in render table
	ResourceTable.ptr = HeapStart.ptr + (DispatchResourceTableStart + INDEX_TLAS) * DescriptorIncSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
	D3D12_SHADER_RESOURCE_VIEW_DESC ASSRVDesc = {};
	ASSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	ASSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	ASSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	ASSRVDesc.RaytracingAccelerationStructure.Location = TLASRes->GetGPUVirtualAddress();

	// https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html
	// When creating descriptor heap based acceleration structure SRVs, 
	// the resource parameter must be NULL, as the memory location comes 
	// as a GPUVA from the view description (D3D12_RAYTRACING_ACCELERATION_STRUCTURE_SRV) 
	// shown below. E.g. CreateShaderResourceView(NULL,pViewDesc).		;
	D3dDevice->CreateShaderResourceView(nullptr, &ASSRVDesc, ResourceTable);
}

const int32 NumExportNames = 10;
const LPCWSTR ExportNames[NumExportNames] =
{
	L"MyRayGenShader",
	L"RayMiss",
	L"RayClosestHit",
	L"RayAnyHit",
	L"MyGlobalRootSignature",
	L"MyLocalRootSignature",
	L"MyHitGroup",
	L"MyLocalRootSignatureAssociation",
	L"MyShaderConfig",
	L"MyPipelineConfig"
};
const LPCWSTR HitGroupName = ExportNames[6];
void TRTXTest::CreateRaytracingPipelineObject()
{
	// Load Shader Code
	const TString ShaderName = "S_Pathtracer.cso";
	TStream ShaderCode;
	TFile File;
	if (File.Open(ShaderName, EFA_READ))
	{
		ShaderCode.Put(File);
		File.Close();
	}
	else
	{
		_LOG(Fatal, "Failed to load shader code [%s].\n", ShaderName.c_str());
	}

	// Create Global Root Signature
	ID3D12RootSignatureDeserializer* RSDeserializer = nullptr;
	VALIDATE_HRESULT(D3D12CreateRootSignatureDeserializer(ShaderCode.GetBuffer(),
		ShaderCode.GetLength(),
		__uuidof(ID3D12RootSignatureDeserializer),
		reinterpret_cast<void**>(&RSDeserializer)));
	const D3D12_ROOT_SIGNATURE_DESC* RSDesc = RSDeserializer->GetRootSignatureDesc();

	ComPtr<ID3DBlob> pOutBlob, pErrorBlob;
	VALIDATE_HRESULT(D3D12SerializeRootSignature(RSDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		pOutBlob.GetAddressOf(), pErrorBlob.GetAddressOf()));

	VALIDATE_HRESULT(D3dDevice->CreateRootSignature(0, pOutBlob->GetBufferPointer(), pOutBlob->GetBufferSize(), IID_PPV_ARGS(&GlobalRS)));
	GlobalRS->SetName(L"GlobalRS");


	// Create Rtx Pipeline state object
	{
		TVector<D3D12_STATE_SUBOBJECT> SubObjects;
		SubObjects.reserve(10);
		D3D12_STATE_SUBOBJECT SubObject;

		// Dxil library
		SubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;

		D3D12_DXIL_LIBRARY_DESC DxilLibDesc;
		SubObject.pDesc = &DxilLibDesc;
		DxilLibDesc.DXILLibrary.pShaderBytecode = ShaderCode.GetBuffer();
		DxilLibDesc.DXILLibrary.BytecodeLength = uint32(ShaderCode.GetLength());

		TVector<D3D12_EXPORT_DESC> ExportDesc;
		ExportDesc.resize(NumExportNames);
		for (int32 i = 0; i < NumExportNames; i++)
		{
			ExportDesc[i].Name = ExportNames[i];
			ExportDesc[i].Flags = D3D12_EXPORT_FLAG_NONE;
			ExportDesc[i].ExportToRename = nullptr;
		}
		DxilLibDesc.NumExports = NumExportNames;
		DxilLibDesc.pExports = ExportDesc.data();
		SubObjects.push_back(SubObject);

		// Create the state
		D3D12_STATE_OBJECT_DESC Desc;
		Desc.NumSubobjects = (uint32)SubObjects.size();
		Desc.pSubobjects = SubObjects.data();
		Desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

		VALIDATE_HRESULT(DXRDevice->CreateStateObject(&Desc, IID_PPV_ARGS(&RTXStateObject)));
		RTXStateObject->SetName(L"RTXStateObject");
	}
}
void TRTXTest::LoadMeshBuffer()
{
	// Load file
	const TString& MeshName = "Meshes/SM_TV.tasset";
	TStream FileBuffer;
	TFile File;
	if (File.Open(MeshName, EFA_READ))
	{
		FileBuffer.Put(File);
		File.Close();
	}
	else
	{
		_LOG(Fatal, "Failed to load mesh [%s].\n", MeshName.c_str());
	}

	// Parsing file
	const int8* Buffer = FileBuffer.GetBuffer();
	int32 Pos = 0;
	TResfileHeader* FileHeader = (TResfileHeader*)(Buffer + Pos);
	if (FileHeader->Version != TIRES_VERSION_MAINFILE)
	{
		TI_ASSERT(0);
	}
	Pos += TMath::Align4((int32)sizeof(TResfileHeader));

	const int32* StringOffsets = (int32*)(Buffer + FileHeader->StringOffset);
	TI_ASSERT(FileHeader->ChunkCount == 1);
	const TResfileChunkHeader* ChunkHeader = (const TResfileChunkHeader*)(Buffer + Pos);
	TI_ASSERT(ChunkHeader->Version == TIRES_VERSION_CHUNK_MESH);
	TI_ASSERT(ChunkHeader->ElementCount == 1);

	const int8* ChunkStart = (const int8*)ChunkHeader;

	// Load meshes
	const int8* MeshDataStart = (const int8*)(ChunkStart + TMath::Align4((int32)sizeof(TResfileChunkHeader)));
	const int8* SectionDataStart = (const int8*)(MeshDataStart + TMath::Align4((int32)sizeof(THeaderMesh)));

	const THeaderMesh* MeshHeader = (const THeaderMesh*)(MeshDataStart);
	const THeaderMeshSection* HeaderSections = (const THeaderMeshSection*)(SectionDataStart);

	TI_ASSERT(MeshHeader->Sections > 0);
	MBVertexCount = MeshHeader->VertexCount;
	MBVertexStride = TMeshBuffer::GetStrideFromFormat(MeshHeader->VertexFormat);
	MBIndexCount = MeshHeader->PrimitiveCount * 3;

	const int8* VertexDataStart = MeshDataStart +
		TMath::Align4((int32)sizeof(THeaderMesh)) * 1 +
		sizeof(THeaderMeshSection) * MeshHeader->Sections +
		0 * sizeof(uint32) +
		sizeof(THeaderCollisionSet);

	// Load vertex data and index data
	const int32 IndexStride = (MeshHeader->IndexType == EIT_16BIT) ? sizeof(uint16) : sizeof(uint32);
	const int32 VertexStride = TMeshBuffer::GetStrideFromFormat(MeshHeader->VertexFormat);
	//const int8* VertexData = VertexDataStart;
	const int8* IndexDataStart = VertexDataStart + TMath::Align4(MeshHeader->VertexCount * VertexStride);
	const int8* ClusterData = IndexDataStart + TMath::Align4(MeshHeader->PrimitiveCount * 3 * IndexStride);

	// Create Vertex Buffer And Index Buffer
	// Create the vertex buffer resource in the GPU's default heap and copy vertex data into it using the upload heap.
	// The upload resource must not be released until after the GPU has finished using it.

	const int32 BufferSize = MeshHeader->VertexCount * VertexStride;
	CD3DX12_HEAP_PROPERTIES DefaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC VertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(BufferSize);

	VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
		&DefaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&VertexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&VertexBuffer)));

	CD3DX12_HEAP_PROPERTIES UploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
		&UploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&VertexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&VertexBufferUpload)));

	VertexBuffer->SetName(L"SM_TV-VB");
	VertexBufferUpload->SetName(L"SM_TV-VBUpload");

	// Upload the vertex buffer to the GPU.
	{
		D3D12_SUBRESOURCE_DATA VertexData = {};
		VertexData.pData = reinterpret_cast<const uint8*>(VertexDataStart);
		VertexData.RowPitch = BufferSize;
		VertexData.SlicePitch = VertexData.RowPitch;

		UpdateSubresources(GraphicsCommandList.Get(), VertexBuffer.Get(), VertexBufferUpload.Get(), 0, 0, 1, &VertexData);

		D3D12_RESOURCE_BARRIER Barrier = {};
		Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		Barrier.Transition.pResource = VertexBuffer.Get();
		Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		GraphicsCommandList->ResourceBarrier(1, &Barrier);
	}

	const uint32 IndexBufferSize = (MeshHeader->PrimitiveCount * 3 * IndexStride);

	// Create the index buffer resource in the GPU's default heap and copy index data into it using the upload heap.
	// The upload resource must not be released until after the GPU has finished using it.

	CD3DX12_RESOURCE_DESC IndexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize);

	VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
		&DefaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&IndexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&IndexBuffer)));
	VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
		&UploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&IndexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&IndexBufferUpload)));

	IndexBuffer->SetName(L"SM_TV-IB");
	IndexBufferUpload->SetName(L"SM_TV-IBUpload");
	// Upload the index buffer to the GPU.
	{
		D3D12_SUBRESOURCE_DATA IndexData = {};
		IndexData.pData = reinterpret_cast<const uint8*>(IndexDataStart);
		IndexData.RowPitch = IndexBufferSize;
		IndexData.SlicePitch = IndexData.RowPitch;

		UpdateSubresources(GraphicsCommandList.Get(), IndexBuffer.Get(), IndexBufferUpload.Get(), 0, 0, 1, &IndexData);

		D3D12_RESOURCE_BARRIER Barrier = {};
		Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		Barrier.Transition.pResource = IndexBuffer.Get();
		Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		GraphicsCommandList->ResourceBarrier(1, &Barrier);

		//DestState |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}

	//FlushGraphicsBarriers(GraphicsCommandList.Get());
}
void TRTXTest::BuildAccelerationStructures()
{
	// ================= BLAS =================
	{
		// Create Geometry Desc
		D3D12_RAYTRACING_GEOMETRY_DESC GeometryDesc = {};
		GeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		GeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		GeometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
		GeometryDesc.Triangles.IndexBuffer = IndexBuffer->GetGPUVirtualAddress();
		GeometryDesc.Triangles.IndexCount = MBIndexCount;

		GeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;	// Position always be RGB32F
		GeometryDesc.Triangles.VertexBuffer.StartAddress = VertexBuffer->GetGPUVirtualAddress();
		GeometryDesc.Triangles.VertexBuffer.StrideInBytes = MBVertexStride;
		GeometryDesc.Triangles.VertexCount = MBVertexCount;

		GeometryDesc.Triangles.Transform3x4 = NULL;

		// Get the size requirements for the scratch and AS buffers.
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO PrebuildInfo = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC BottomLevelBuildDesc = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& BottomLevelInputs = BottomLevelBuildDesc.Inputs;
		BottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		BottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		BottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		BottomLevelInputs.NumDescs = 1;
		BottomLevelInputs.pGeometryDescs = &GeometryDesc;

		DXRDevice->GetRaytracingAccelerationStructurePrebuildInfo(&BottomLevelInputs, &PrebuildInfo);
		TI_ASSERT(PrebuildInfo.ResultDataMaxSizeInBytes > 0);

		// Allocate resource for BLAS
		TI_ASSERT(BLASRes == nullptr);
		auto UploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto ASBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(PrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		VALIDATE_HRESULT(DXRDevice->CreateCommittedResource(
			&UploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&ASBufferDesc,
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			nullptr,
			IID_PPV_ARGS(&BLASRes)));
		BLASRes->SetName(L"BLASRes");

		TI_ASSERT(BLASScratch == nullptr);
		auto ScratchBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(PrebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		VALIDATE_HRESULT(DXRDevice->CreateCommittedResource(
			&UploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&ScratchBufferDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&BLASScratch)));
		BLASScratch->SetName(L"BLASScratch");

		// Build bottom layer AS
		BottomLevelBuildDesc.ScratchAccelerationStructureData = BLASScratch->GetGPUVirtualAddress();
		BottomLevelBuildDesc.DestAccelerationStructureData = BLASRes->GetGPUVirtualAddress();

		// https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html
		// Array of vertex indices.If NULL, triangles are non - indexed.Just as with graphics, 
		// the address must be aligned to the size of IndexFormat. The memory pointed to must 
		// be in state D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE.Note that if an app wants 
		// to share index buffer inputs between graphics input assemblerand raytracing 
		// acceleration structure build input, it can always put a resource into a combination 
		// of read states simultaneously, 
		// e.g.D3D12_RESOURCE_STATE_INDEX_BUFFER | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
		ID3D12DescriptorHeap* Heap = DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Get();
		DXRCommandList->SetDescriptorHeaps(1, &Heap);
		DXRCommandList->BuildRaytracingAccelerationStructure(&BottomLevelBuildDesc, 0, nullptr);
	}

	// ================= TLAS =================
	{
		FMatrix3x4 Mat3x4;
		Mat3x4.SetTranslation(vector3df(-2.78112245f, 1.78132439f, 0.970187485f));
		Mat3x4[0] = 1.f;
		Mat3x4[1] = 0.f;
		Mat3x4[2] = 0.f;

		Mat3x4[4] = 0.f;
		Mat3x4[5] = 1.f;
		Mat3x4[6] = 0.f;

		Mat3x4[8] = 0.f;
		Mat3x4[9] = 0.f;
		Mat3x4[10] = 1.f;

		D3D12_RAYTRACING_INSTANCE_DESC Desc;
		memcpy(Desc.Transform, Mat3x4.Data(), sizeof(float) * 3 * 4);
		Desc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
		Desc.InstanceID = 0;
		Desc.InstanceMask = 1;
		Desc.InstanceContributionToHitGroupIndex = 0;
		Desc.AccelerationStructure = BLASRes->GetGPUVirtualAddress();

		// Get the size requirements for the scratch and AS buffers.
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO PrebuildInfo = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC TopLevelBuildDesc = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& TopLevelInputs = TopLevelBuildDesc.Inputs;
		TopLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		TopLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		TopLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		TopLevelInputs.NumDescs = 1;

		DXRDevice->GetRaytracingAccelerationStructurePrebuildInfo(&TopLevelInputs, &PrebuildInfo);
		TI_ASSERT(PrebuildInfo.ResultDataMaxSizeInBytes > 0);

		// Allocate resource for TLAS
		TI_ASSERT(TLASRes == nullptr);
		auto DefaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto TLASBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(PrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		VALIDATE_HRESULT(DXRDevice->CreateCommittedResource(
			&DefaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&TLASBufferDesc,
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			nullptr,
			IID_PPV_ARGS(&TLASRes)));
		TLASRes->SetName(L"TLASRes");

		TI_ASSERT(TLASScratch == nullptr);
		auto ScratchBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(PrebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		VALIDATE_HRESULT(DXRDevice->CreateCommittedResource(
			&DefaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&ScratchBufferDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&TLASScratch)));
		TLASScratch->SetName(L"TLASScratch");

		// Create Instance Resource
		const uint32 InstanceBufferSize = 1 * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
		auto InstanceBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(InstanceBufferSize, D3D12_RESOURCE_FLAG_NONE);
		auto UploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		VALIDATE_HRESULT(DXRDevice->CreateCommittedResource(
			&UploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&InstanceBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&TLASInstance)));
		TLASInstance->SetName(L"TLASInstance");
		D3D12_RAYTRACING_INSTANCE_DESC* pInstanceDesc;
		TLASInstance->Map(0, nullptr, (void**)&pInstanceDesc);
		memcpy(pInstanceDesc, &Desc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
		TLASInstance->Unmap(0, nullptr);

		// Build top layer AS
		TopLevelInputs.InstanceDescs = TLASInstance->GetGPUVirtualAddress();
		TopLevelBuildDesc.ScratchAccelerationStructureData = TLASScratch->GetGPUVirtualAddress();
		TopLevelBuildDesc.DestAccelerationStructureData = TLASRes->GetGPUVirtualAddress();

		// https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html
		// Array of vertex indices.If NULL, triangles are non - indexed.Just as with graphics, 
		// the address must be aligned to the size of IndexFormat. The memory pointed to must 
		// be in state D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE.Note that if an app wants 
		// to share index buffer inputs between graphics input assemblerand raytracing 
		// acceleration structure build input, it can always put a resource into a combination 
		// of read states simultaneously, 
		// e.g.D3D12_RESOURCE_STATE_INDEX_BUFFER | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
		ID3D12DescriptorHeap* Heap = DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Get();
		DXRCommandList->SetDescriptorHeaps(1, &Heap);
		DXRCommandList->BuildRaytracingAccelerationStructure(&TopLevelBuildDesc, 0, nullptr);
	}
}
void TRTXTest::BuildShaderTables()
{
	// Build Shader Table
	{
		ComPtr<ID3D12StateObjectProperties> StateObjectProperties;
		VALIDATE_HRESULT(RTXStateObject.As(&StateObjectProperties));

		// Get shader identifiers
		TI_TODO("Use correct raygen/miss/hitgroup name, for now, use ExportNames[0,1] for raygen and miss, HitGroupName for hitgroup");
		void* RayGenShaderId = StateObjectProperties->GetShaderIdentifier(ExportNames[0]);
		void* MissShaderId = StateObjectProperties->GetShaderIdentifier(ExportNames[1]);
		void* HitgroupShaderId = StateObjectProperties->GetShaderIdentifier(HitGroupName);

		// DispatchRays: 
		// pDesc->MissShaderTable.StartAddress must be aligned to 64 bytes(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT) 
		// and .StrideInBytes must be aligned to 32 bytes(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT)

		uint32 ShaderTableSize = 0;
		// Ray gen
		RayGenShaderOffsetAndSize.X = ShaderTableSize;
		RayGenShaderOffsetAndSize.Y = TMath::Align(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		ShaderTableSize += TMath::Align(RayGenShaderOffsetAndSize.Y, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		// Miss
		MissShaderOffsetAndSize.X = ShaderTableSize;
		MissShaderOffsetAndSize.Y = TMath::Align(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		ShaderTableSize += TMath::Align(MissShaderOffsetAndSize.Y, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		// Hit Group
		HitGroupOffsetAndSize.X = ShaderTableSize;
		HitGroupOffsetAndSize.Y = TMath::Align(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		ShaderTableSize += TMath::Align(HitGroupOffsetAndSize.Y, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		TI_TODO("Calc shader table size with shader parameters");

		TI_ASSERT(ShaderTable == nullptr);
		CD3DX12_RESOURCE_DESC ConstantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(ShaderTableSize);
		CD3DX12_HEAP_PROPERTIES UploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		VALIDATE_HRESULT(DXRDevice->CreateCommittedResource(
			&UploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&ConstantBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&ShaderTable)));
		ShaderTable->SetName(L"ShaderTable");
		// Build shader table data
		uint8* Data = ti_new uint8[ShaderTableSize];
		memset(Data, 0, ShaderTableSize);
		uint8* pData = Data;
		memcpy(pData, RayGenShaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		pData += TMath::Align(RayGenShaderOffsetAndSize.Y, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		memcpy(pData, MissShaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		pData += TMath::Align(MissShaderOffsetAndSize.Y, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		memcpy(pData, HitgroupShaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		pData += TMath::Align(HitGroupOffsetAndSize.Y, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		// Map the constant buffers.
		uint8* MappedConstantBuffer = nullptr;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		VALIDATE_HRESULT(ShaderTable->Map(0, &readRange, reinterpret_cast<void**>(&MappedConstantBuffer)));
		memcpy(MappedConstantBuffer, Data, ShaderTableSize);
		ShaderTable->Unmap(0, nullptr);
		ti_delete[] Data;
	}
}

void TRTXTest::Tick()
{
}

void TRTXTest::Render()
{
	BeginFrame();

	if (VertexBuffer == nullptr)
	{
		CreateResources();
	}
	UpdateCamInfo();
	DispatchRays();
	CopyRaytracingOutputToBackbuffer();

	EndFrame();
}

void TRTXTest::BeginFrame()
{
	// Reset command list
	VALIDATE_HRESULT(GraphicsAllocators[CurrentFrame]->Reset());
	VALIDATE_HRESULT(GraphicsCommandList->Reset(GraphicsAllocators[CurrentFrame].Get(), nullptr));

	// Set the descriptor heaps to be used by this frame.
	ID3D12DescriptorHeap* ppHeaps[] = { DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Get() };
	GraphicsCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// Set the viewport and scissor rectangle.
	D3D12_VIEWPORT ViewportDx = { 0, 0, WinWidth, WinHeight, 0.f, 1.f };
	GraphicsCommandList->RSSetViewports(1, &ViewportDx);
	D3D12_RECT ScissorRect = { 0, 0, WinWidth, WinHeight };
	GraphicsCommandList->RSSetScissorRects(1, &ScissorRect);
}

void TRTXTest::EndFrame()
{
	// Indicate that the render target will now be used to present when the command list is done executing.
	// Already is STATE_PRESENT
	//D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(BackBufferRTs[CurrentFrame].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	//GraphicsCommandList->ResourceBarrier(1, &barrier);
	GraphicsCommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { GraphicsCommandList.Get() };
	GraphicsCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

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

		// MoveToNextFrame();
		// Schedule a Signal command in the queue.
		const uint64 currentFenceValue = FenceValues[CurrentFrame];
		VALIDATE_HRESULT(GraphicsCommandQueue->Signal(GraphicsFence.Get(), currentFenceValue));

		// Advance the frame index.
		CurrentFrame = SwapChain->GetCurrentBackBufferIndex();

		// Check to see if the next frame is ready to start.
		if (GraphicsFence->GetCompletedValue() < FenceValues[CurrentFrame])
		{
			VALIDATE_HRESULT(GraphicsFence->SetEventOnCompletion(FenceValues[CurrentFrame], FenceEvent));
			WaitForSingleObjectEx(FenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		FenceValues[CurrentFrame] = currentFenceValue + 1;
	}
}

void TRTXTest::UpdateCamInfo()
{
}

void TRTXTest::DispatchRays()
{
	TVector<D3D12_RESOURCE_BARRIER> Barriers;
	Barriers.resize(1);

	// Barrier for OutputTexture
	Barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(OutputTexture.Get(), OutputTextureState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	GraphicsCommandList->ResourceBarrier(1, Barriers.data());
	OutputTextureState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	// Set Rtx Pipeline
	DXRCommandList->SetPipelineState1(RTXStateObject.Get());

	// Bind Global root signature
	DXRCommandList->SetComputeRootSignature(GlobalRS.Get());

	D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor = DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetGPUDescriptorHandleForHeapStart();
	GPUDescriptor.ptr += DispatchResourceTableStart * DescriptorIncSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
	DXRCommandList->SetComputeRootDescriptorTable(0, GPUDescriptor);

	//RHI->SetComputeResourceTable(0, ResourceTable);
	DXRCommandList->SetComputeRootConstantBufferView(1, ConstantBuffer->GetGPUVirtualAddress());
	//RHI->SetComputeConstantBuffer(1, UB_Pathtracer->UniformBuffer);

	D3D12_DISPATCH_RAYS_DESC RaytraceDesc = {};
	RaytraceDesc.Width = WinWidth;
	RaytraceDesc.Height = WinHeight;
	RaytraceDesc.Depth = 1;

	// RayGen is the first entry in the shader-table
	RaytraceDesc.RayGenerationShaderRecord.StartAddress =
		ShaderTable.Get()->GetGPUVirtualAddress() + RayGenShaderOffsetAndSize.X;
	RaytraceDesc.RayGenerationShaderRecord.SizeInBytes = RayGenShaderOffsetAndSize.Y;

	// Miss is the second entry in the shader-table
	RaytraceDesc.MissShaderTable.StartAddress =
		ShaderTable.Get()->GetGPUVirtualAddress() + MissShaderOffsetAndSize.X;
	RaytraceDesc.MissShaderTable.StrideInBytes = MissShaderOffsetAndSize.Y;
	RaytraceDesc.MissShaderTable.SizeInBytes = MissShaderOffsetAndSize.Y;   // Only a s single miss-entry

	// Hit is the third entry in the shader-table
	RaytraceDesc.HitGroupTable.StartAddress =
		ShaderTable.Get()->GetGPUVirtualAddress() + HitGroupOffsetAndSize.X;
	RaytraceDesc.HitGroupTable.StrideInBytes = HitGroupOffsetAndSize.Y;
	RaytraceDesc.HitGroupTable.SizeInBytes = HitGroupOffsetAndSize.Y;

	// Dispatch
	DXRCommandList->DispatchRays(&RaytraceDesc);
	//RHI->TraceRays(RtxPSO, TraceSize);
}

// Copy the raytracing output to the backbuffer.
void TRTXTest::CopyRaytracingOutputToBackbuffer()
{
	ID3D12Resource* RenderTarget = BackBufferRTs[CurrentFrame].Get();

	D3D12_RESOURCE_BARRIER preCopyBarriers[2];
	preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RenderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
	preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(OutputTexture.Get(), OutputTextureState, D3D12_RESOURCE_STATE_COPY_SOURCE);
	DXRCommandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);
	OutputTextureState = D3D12_RESOURCE_STATE_COPY_SOURCE;

	DXRCommandList->CopyResource(RenderTarget, OutputTexture.Get());

	D3D12_RESOURCE_BARRIER postCopyBarriers[1];
	postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RenderTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
	DXRCommandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}