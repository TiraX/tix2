/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "RTXTest2.h"
#include "Dx12/d3dx12.h"

// link libraries
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "d3dcompiler.lib")

const int32 WinWidth = 1600;
const int32 WinHeight = 900;

const DXGI_FORMAT OutputTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;// DXGI_FORMAT_B8G8R8A8_UNORM;// DXGI_FORMAT_R8G8B8A8_UNORM;
const DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;// DXGI_FORMAT_B8G8R8A8_UNORM;
const DXGI_FORMAT DepthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

static const int enable = 0;

class GpuUploadBuffer
{
public:
	ComPtr<ID3D12Resource> GetResource() { return m_resource; }

public:
	ComPtr<ID3D12Resource> m_resource;

	GpuUploadBuffer() {}
	~GpuUploadBuffer()
	{
		if (m_resource.Get())
		{
			m_resource->Unmap(0, nullptr);
		}
	}

	void Allocate(ID3D12Device* device, UINT bufferSize, LPCWSTR resourceName = nullptr)
	{
		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
		VALIDATE_HRESULT(device->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_resource)));
		m_resource->SetName(resourceName);
	}

	uint8_t* MapCpuWriteOnly()
	{
		uint8_t* mappedData;
		// We don't unmap this until the app closes. Keeping buffer mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		VALIDATE_HRESULT(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
		return mappedData;
	}
};


// Shader record = {{Shader ID}, {RootArguments}}
class ShaderRecord
{
public:
	ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize) :
		shaderIdentifier(pShaderIdentifier, shaderIdentifierSize)
	{
	}

	ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize, void* pLocalRootArguments, UINT localRootArgumentsSize) :
		shaderIdentifier(pShaderIdentifier, shaderIdentifierSize),
		localRootArguments(pLocalRootArguments, localRootArgumentsSize)
	{
	}

	void CopyTo(void* dest) const
	{
		uint8_t* byteDest = static_cast<uint8_t*>(dest);
		memcpy(byteDest, shaderIdentifier.ptr, shaderIdentifier.size);
		if (localRootArguments.ptr)
		{
			memcpy(byteDest + shaderIdentifier.size, localRootArguments.ptr, localRootArguments.size);
		}
	}

	struct PointerWithSize {
		void* ptr;
		UINT size;

		PointerWithSize() : ptr(nullptr), size(0) {}
		PointerWithSize(void* _ptr, UINT _size) : ptr(_ptr), size(_size) {};
	};
	PointerWithSize shaderIdentifier;
	PointerWithSize localRootArguments;
};

// Shader table = {{ ShaderRecord 1}, {ShaderRecord 2}, ...}
class ShaderTable : public GpuUploadBuffer
{
public:
	uint8_t* m_mappedShaderRecords;
	UINT m_shaderRecordSize;

	// Debug support
	std::wstring m_name;
	std::vector<ShaderRecord> m_shaderRecords;

	ShaderTable() {}
public:
	ShaderTable(ID3D12Device* device, UINT numShaderRecords, UINT shaderRecordSize, LPCWSTR resourceName = nullptr)
		: m_name(resourceName)
	{
		m_shaderRecordSize = TMath::Align(shaderRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		m_shaderRecords.reserve(numShaderRecords);
		UINT bufferSize = numShaderRecords * m_shaderRecordSize;
		Allocate(device, bufferSize, resourceName);
		m_mappedShaderRecords = MapCpuWriteOnly();
	}

	void push_back(const ShaderRecord& shaderRecord)
	{
		VALIDATE_HRESULT(m_shaderRecords.size() < m_shaderRecords.capacity());
		m_shaderRecords.push_back(shaderRecord);
		shaderRecord.CopyTo(m_mappedShaderRecords);
		m_mappedShaderRecords += m_shaderRecordSize;
	}

	UINT GetShaderRecordSize() { return m_shaderRecordSize; }

	// Pretty-print the shader records.
	void DebugPrint(std::unordered_map<void*, std::wstring> shaderIdToStringMap)
	{
		std::wstringstream wstr;
		wstr << L"|--------------------------------------------------------------------\n";
		wstr << L"|Shader table - " << m_name.c_str() << L": "
			<< m_shaderRecordSize << L" | "
			<< m_shaderRecords.size() * m_shaderRecordSize << L" bytes\n";

		for (UINT i = 0; i < m_shaderRecords.size(); i++)
		{
			wstr << L"| [" << i << L"]: ";
			wstr << shaderIdToStringMap[m_shaderRecords[i].shaderIdentifier.ptr] << L", ";
			wstr << m_shaderRecords[i].shaderIdentifier.size << L" + " << m_shaderRecords[i].localRootArguments.size << L" bytes \n";
		}
		wstr << L"|--------------------------------------------------------------------\n";
		wstr << L"\n";
		OutputDebugStringW(wstr.str().c_str());
	}
};

//------------------------------------------------------------------------------------------------
// All arrays must be populated (e.g. by calling GetCopyableFootprints)
//static uint64 UpdateSubresources(
//	_In_ ID3D12GraphicsCommandList* pCmdList,
//	_In_ ID3D12Resource* pDestinationResource,
//	_In_ ID3D12Resource* pIntermediate,
//	_In_range_(0, D3D12_REQ_SUBRESOURCES) uint32 FirstSubresource,
//	_In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) uint32 NumSubresources,
//	uint64 RequiredSize,
//	_In_reads_(NumSubresources) const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
//	_In_reads_(NumSubresources) const uint32* pNumRows,
//	_In_reads_(NumSubresources) const uint64* pRowSizesInBytes,
//	_In_reads_(NumSubresources) const D3D12_SUBRESOURCE_DATA* pSrcData)
//{
//	// Minor validation
//	D3D12_RESOURCE_DESC IntermediateDesc = pIntermediate->GetDesc();
//	D3D12_RESOURCE_DESC DestinationDesc = pDestinationResource->GetDesc();
//	if (IntermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
//		IntermediateDesc.Width < RequiredSize + pLayouts[0].Offset ||
//		RequiredSize >(SIZE_T) - 1 ||
//		(DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER &&
//			(FirstSubresource != 0 || NumSubresources != 1)))
//	{
//		return 0;
//	}
//
//	uint8* pData;
//	HRESULT hr = pIntermediate->Map(0, nullptr, reinterpret_cast<void**>(&pData));
//	if (FAILED(hr))
//	{
//		return 0;
//	}
//
//	for (uint32 i = 0; i < NumSubresources; ++i)
//	{
//		if (pRowSizesInBytes[i] > (SIZE_T)-1) return 0;
//		D3D12_MEMCPY_DEST DestData = { pData + pLayouts[i].Offset, pLayouts[i].Footprint.RowPitch, pLayouts[i].Footprint.RowPitch * pNumRows[i] };
//		MemcpySubresource(&DestData, &pSrcData[i], (SIZE_T)pRowSizesInBytes[i], pNumRows[i], pLayouts[i].Footprint.Depth);
//	}
//	pIntermediate->Unmap(0, nullptr);
//
//	if (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
//	{
//		CD3DX12_BOX SrcBox(uint32(pLayouts[0].Offset), uint32(pLayouts[0].Offset + pLayouts[0].Footprint.Width));
//		pCmdList->CopyBufferRegion(
//			pDestinationResource, 0, pIntermediate, pLayouts[0].Offset, pLayouts[0].Footprint.Width);
//	}
//	else
//	{
//		for (uint32 i = 0; i < NumSubresources; ++i)
//		{
//			CD3DX12_TEXTURE_COPY_LOCATION Dst(pDestinationResource, i + FirstSubresource);
//			CD3DX12_TEXTURE_COPY_LOCATION Src(pIntermediate, pLayouts[i]);
//			pCmdList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
//		}
//	}
//	return RequiredSize;
//}

//------------------------------------------------------------------------------------------------
// Heap-allocating UpdateSubresources implementation
static uint64 UpdateSubresources(
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

TRTXTest2::TRTXTest2()
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


	float border = 0.1f;
	float Aspect = float(WinWidth) / float(WinHeight);
	{
		m_rayGenCB.stencil =
		{
			-1 + border / Aspect, -1 + border,
			 1 - border / Aspect, 1.0f - border
		};
		m_rayGenCB.viewport = { -1.0f, -1.0f, 1.0f, 1.0f };
	}
}

TRTXTest2::~TRTXTest2()
{
}

void TRTXTest2::Run()
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

void TRTXTest2::Init()
{
	CreateAWindow();
	InitD3D12();
}


static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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

void TRTXTest2::CreateAWindow()
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

bool TRTXTest2::IsRunning()
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

void TRTXTest2::InitD3D12()
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
#if defined(TIX_DEBUG)
		D3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		D3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif
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

	if (enable)
	{
		HeapDesc.NumDescriptors = MaxDescriptorCount[D3D12_DESCRIPTOR_HEAP_TYPE_DSV];
		HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		HeapDesc.Flags = k_HEAP_CREATE_FLAGS[D3D12_DESCRIPTOR_HEAP_TYPE_DSV];
		HeapDesc.NodeMask = 0;
		VALIDATE_HRESULT(D3dDevice->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_DSV])));
		DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_DSV]->SetName(L"HeapDSV");
		DescriptorIncSize[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

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
	VALIDATE_HRESULT(D3dDevice->CreateFence(FenceValues[CurrentFrame], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&GraphicsFence)));
	GraphicsFence->SetName(L"GraphicsFence");

	FenceValues[CurrentFrame]++;

	FenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));
	if (!FenceEvent.IsValid())
	{
		Hr = HRESULT_FROM_WIN32(GetLastError());
		TI_ASSERT(0);
	}

	WaitForGPU();
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
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;// 0;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		ComPtr<IDXGISwapChain1> swapChain;
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { 0 };
		fsSwapChainDesc.Windowed = TRUE;

		hr = DxgiFactory->CreateSwapChainForHwnd(
			GraphicsCommandQueue.Get(),								// Swap chains need a reference to the command queue in DirectX 12.
			HWnd,
			&swapChainDesc,
			&fsSwapChainDesc,
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
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = BackBufferFormat;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			BackBufferDescriptors[n].ptr = DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_RTV]->GetCPUDescriptorHandleForHeapStart().ptr + n * DescriptorIncSize[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
			VALIDATE_HRESULT(SwapChain->GetBuffer(n, IID_PPV_ARGS(&BackBufferRTs[n])));
			D3dDevice->CreateRenderTargetView(BackBufferRTs[n].Get(), &rtvDesc, BackBufferDescriptors[n]);

			WCHAR name[32];
			if (swprintf_s(name, L"BackBufferRTs[%u]", n) > 0)
			{
				BackBufferRTs[n].Get()->SetName(name);
			}
		}
		CurrentFrame = SwapChain->GetCurrentBackBufferIndex();
	}

	// Create a depth stencil and view.
	if (enable)
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

	// Set the 3D rendering viewport and scissor rectangle to target the entire window.
	Viewport.TopLeftX = Viewport.TopLeftY = 0.f;
	Viewport.Width = static_cast<float>(WinWidth);
	Viewport.Height = static_cast<float>(WinHeight);
	Viewport.MinDepth = D3D12_MIN_DEPTH;
	Viewport.MaxDepth = D3D12_MAX_DEPTH;

	ScissorRect.left = ScissorRect.top = 0;
	ScissorRect.right = WinWidth;
	ScissorRect.bottom = WinHeight;

	// Init raytracing
	VALIDATE_HRESULT(D3dDevice->QueryInterface(IID_PPV_ARGS(&DXRDevice)));
	VALIDATE_HRESULT(GraphicsCommandList->QueryInterface(IID_PPV_ARGS(&DXRCommandList)));

	// Describe and create a shader resource view (SRV) heap for the texture.
	HeapDesc.NumDescriptors = MaxDescriptorCount[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	HeapDesc.Flags = k_HEAP_CREATE_FLAGS[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
	HeapDesc.NodeMask = 0;
	VALIDATE_HRESULT(D3dDevice->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV])));
	DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->SetName(L"HeapCBV_SRV_UAV");
	DescriptorIncSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


}

void TRTXTest2::CreateRootSignatures()
{

	// Load Shader Code
	const TString ShaderName = "Raytracing.cso";
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
	// A unique global root signature is defined in hlsl library g_pRaytracing. For such scenario we can create 
	// compute root signature can directly from the library bytecode, using CreateRootSignature API. 
	VALIDATE_HRESULT(D3dDevice->CreateRootSignature(0, ShaderCode.GetBuffer(), ShaderCode.GetLength(), IID_PPV_ARGS(&GlobalRS)));
	GlobalRS->SetName(L"GlobalRS");


	//// Global Root Signature
	//// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
	//{
	//	ComPtr<ID3DBlob> blob;
	//	ComPtr<ID3DBlob> error;

	//	CD3DX12_DESCRIPTOR_RANGE UAVDescriptor;
	//	UAVDescriptor.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	//	CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignatureParams::Count];
	//	rootParameters[GlobalRootSignatureParams::OutputViewSlot].InitAsDescriptorTable(1, &UAVDescriptor);
	//	rootParameters[GlobalRootSignatureParams::AccelerationStructureSlot].InitAsShaderResourceView(0);
	//	CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
	//	//SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &GlobalRS);
	//	VALIDATE_HRESULT(D3D12SerializeRootSignature(&globalRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error));
	//	VALIDATE_HRESULT(D3dDevice->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&GlobalRS)));
	//}

	//// Local Root Signature
	//// This is a root signature that enables a shader to have unique arguments that come from shader tables.
	//{
	//	ComPtr<ID3DBlob> blob;
	//	ComPtr<ID3DBlob> error;

	//	CD3DX12_ROOT_PARAMETER rootParameters[LocalRootSignatureParams::Count];
	//	rootParameters[LocalRootSignatureParams::ViewportConstantSlot].InitAsConstants(sizeof(RayGenConstantBuffer) / sizeof(uint32), 0, 0);
	//	CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
	//	localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	//	//SerializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &LocalRS);
	//	VALIDATE_HRESULT(D3D12SerializeRootSignature(&localRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error));
	//	VALIDATE_HRESULT(D3dDevice->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&LocalRS)));
	//}
}
typedef uint16 Index;
struct Vertex { float v1, v2, v3; };

void TRTXTest2::BuildGeometry()
{
	Index indices[] =
	{
		0, 1, 2
	};

	float depthValue = 1.0;
	float offset = 0.7f;
	Vertex vertices[] =
	{
		// The sample raytraces in screen space coordinates.
		// Since DirectX screen space coordinates are right handed (i.e. Y axis points down).
		// Define the vertices in counter clockwise order ~ clockwise in left handed.
		{ 0, -offset, depthValue },
		{ -offset, offset, depthValue },
		{ offset, offset, depthValue }
	};

	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));
	VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&VertexBuffer)));
	VertexBuffer->SetName(L"VB");
	void* pMappedData;
	VertexBuffer->Map(0, nullptr, &pMappedData);
	memcpy(pMappedData, vertices, sizeof(vertices));
	VertexBuffer->Unmap(0, nullptr);

	bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices));
	VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&IndexBuffer)));
	IndexBuffer->SetName(L"IB");
	IndexBuffer->Map(0, nullptr, &pMappedData);
	memcpy(pMappedData, indices, sizeof(indices));
	IndexBuffer->Unmap(0, nullptr);
}

void TRTXTest2::CreateResources()
{
	CreateRootSignatures();
	CreateRaytracingPipelineObject();
	BuildGeometry();
	//BuildGeometryTV();
	BuildAccelerationStructures();
	//BuildBLAS();
	//BuildTLAS();
	BuildShaderTablesSample();
	CreateRaytracingOutputResource();
	//InitCamera();
	//CreateOutputTexture();
	////LoadMeshBuffer();
	//LoadMeshBufferExpand();
	////BuildBLAS();
	//BuildBLASExpand();
	//BuildTLAS();
	//BuildShaderTables();
	//CreateShaderParameters();
}

const vector3df CamPos = vector3df(-2.12574f, -1.05096f, 1.22416f);
const vector3df CamTar = vector3df(-4.25856f, 8.56625f, -0.49643f);

void TRTXTest2::InitCamera()
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

void TRTXTest2::CreateOutputTexture()
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

void TRTXTest2::CreateShaderParameters()
{
	// Create constant buffer
	TI_ASSERT(ConstantBuffer == nullptr);
	const int32 CBSize = sizeof(float) * 4 + sizeof(float) * 16;
	CD3DX12_RESOURCE_DESC ConstantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(CBSize, D3D12_RESOURCE_FLAG_NONE, 65536);
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

// Pretty-print a state object tree.
inline void PrintStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc)
{
	std::wstringstream wstr;
	wstr << L"\n";
	wstr << L"--------------------------------------------------------------------\n";
	wstr << L"| D3D12 State Object 0x" << static_cast<const void*>(desc) << L": ";
	if (desc->Type == D3D12_STATE_OBJECT_TYPE_COLLECTION) wstr << L"Collection\n";
	if (desc->Type == D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE) wstr << L"Raytracing Pipeline\n";

	auto ExportTree = [](UINT depth, UINT numExports, const D3D12_EXPORT_DESC* exports)
	{
		std::wostringstream woss;
		for (UINT i = 0; i < numExports; i++)
		{
			woss << L"|";
			if (depth > 0)
			{
				for (UINT j = 0; j < 2 * depth - 1; j++) woss << L" ";
			}
			woss << L" [" << i << L"]: ";
			if (exports[i].ExportToRename) woss << exports[i].ExportToRename << L" --> ";
			woss << exports[i].Name << L"\n";
		}
		return woss.str();
	};

	for (UINT i = 0; i < desc->NumSubobjects; i++)
	{
		wstr << L"| [" << i << L"]: ";
		switch (desc->pSubobjects[i].Type)
		{
		case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE:
			wstr << L"Global Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
			break;
		case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
			wstr << L"Local Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
			break;
		//case D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK:
		//	wstr << L"Node Mask: 0x" << std::hex << std::setfill(L'0') << std::setw(8) << *static_cast<const UINT*>(desc->pSubobjects[i].pDesc) << std::setw(0) << std::dec << L"\n";
		//	break;
		case D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY:
		{
			wstr << L"DXIL Library 0x";
			auto lib = static_cast<const D3D12_DXIL_LIBRARY_DESC*>(desc->pSubobjects[i].pDesc);
			wstr << lib->DXILLibrary.pShaderBytecode << L", " << lib->DXILLibrary.BytecodeLength << L" bytes\n";
			wstr << ExportTree(1, lib->NumExports, lib->pExports);
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION:
		{
			wstr << L"Existing Library 0x";
			auto collection = static_cast<const D3D12_EXISTING_COLLECTION_DESC*>(desc->pSubobjects[i].pDesc);
			wstr << collection->pExistingCollection << L"\n";
			wstr << ExportTree(1, collection->NumExports, collection->pExports);
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
		{
			wstr << L"Subobject to Exports Association (Subobject [";
			auto association = static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
			UINT index = static_cast<UINT>(association->pSubobjectToAssociate - desc->pSubobjects);
			wstr << index << L"])\n";
			for (UINT j = 0; j < association->NumExports; j++)
			{
				wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
			}
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
		{
			wstr << L"DXIL Subobjects to Exports Association (";
			auto association = static_cast<const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
			wstr << association->SubobjectToAssociate << L")\n";
			for (UINT j = 0; j < association->NumExports; j++)
			{
				wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
			}
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
		{
			wstr << L"Raytracing Shader Config\n";
			auto config = static_cast<const D3D12_RAYTRACING_SHADER_CONFIG*>(desc->pSubobjects[i].pDesc);
			wstr << L"|  [0]: Max Payload Size: " << config->MaxPayloadSizeInBytes << L" bytes\n";
			wstr << L"|  [1]: Max Attribute Size: " << config->MaxAttributeSizeInBytes << L" bytes\n";
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
		{
			wstr << L"Raytracing Pipeline Config\n";
			auto config = static_cast<const D3D12_RAYTRACING_PIPELINE_CONFIG*>(desc->pSubobjects[i].pDesc);
			wstr << L"|  [0]: Max Recursion Depth: " << config->MaxTraceRecursionDepth << L"\n";
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP:
		{
			wstr << L"Hit Group (";
			auto hitGroup = static_cast<const D3D12_HIT_GROUP_DESC*>(desc->pSubobjects[i].pDesc);
			wstr << (hitGroup->HitGroupExport ? hitGroup->HitGroupExport : L"[none]") << L")\n";
			wstr << L"|  [0]: Any Hit Import: " << (hitGroup->AnyHitShaderImport ? hitGroup->AnyHitShaderImport : L"[none]") << L"\n";
			wstr << L"|  [1]: Closest Hit Import: " << (hitGroup->ClosestHitShaderImport ? hitGroup->ClosestHitShaderImport : L"[none]") << L"\n";
			wstr << L"|  [2]: Intersection Import: " << (hitGroup->IntersectionShaderImport ? hitGroup->IntersectionShaderImport : L"[none]") << L"\n";
			break;
		}
		}
		wstr << L"|--------------------------------------------------------------------\n";
	}
	wstr << L"\n";
	OutputDebugStringW(wstr.str().c_str());
}

//const int32 NumExportNames = 10;
//const LPCWSTR ExportNames[NumExportNames] =
//{
//	L"MyRayGenShader",
//	L"RayMiss",
//	L"RayClosestHit",
//	L"RayAnyHit",
//	L"MyGlobalRootSignature",
//	L"MyLocalRootSignature",
//	L"MyHitGroup",
//	L"MyLocalRootSignatureAssociation",
//	L"MyShaderConfig",
//	L"MyPipelineConfig"
//}; 

const wchar_t* c_hitGroupName = L"MyHitGroup";
const wchar_t* c_raygenShaderName = L"MyRaygenShader";
const wchar_t* c_closestHitShaderName = L"MyClosestHitShader";
const wchar_t* c_missShaderName = L"MyMissShader";

// Library subobject names
const wchar_t* c_globalRootSignatureName = L"MyGlobalRS";
const wchar_t* c_localRootSignatureName = L"MyLocalRS";
const wchar_t* c_localRootSignatureAssociationName = L"MyLocalRootSignatureAssociation";
const wchar_t* c_shaderConfigName = L"MyShaderConfig";
const wchar_t* c_pipelineConfigName = L"MyPipelineConfig";
const int32 NumExportNames = 9;
const LPCWSTR ExportNames[NumExportNames] =
{
	c_raygenShaderName,
	c_closestHitShaderName,
	c_missShaderName,

	c_globalRootSignatureName,
	c_localRootSignatureName,
	c_localRootSignatureAssociationName,
	c_shaderConfigName,
	c_pipelineConfigName,
	c_hitGroupName
};
const LPCWSTR HitGroupName = c_hitGroupName;
const LPCWSTR RaygenShader = c_raygenShaderName;

void TRTXTest2::CreateRaytracingPipelineObject()
{
	// Load Shader Code
	const TString ShaderName = "Raytracing.cso";
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
	// Create 7 subobjects that combine into a RTPSO:
	// Subobjects need to be associated with DXIL exports (i.e. shaders) either by way of default or explicit associations.
	// Default association applies to every exported shader entrypoint that doesn't have any of the same type of subobject associated with it.
	// This simple sample utilizes default shader association except for local root signature subobject
	// which has an explicit association specified purely for demonstration purposes.
	// 1 - DXIL library
	// 1 - Triangle hit group
	// 1 - Shader config
	// 2 - Local root signature and association
	// 1 - Global root signature
	// 1 - Pipeline config


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

		// Create the state object.
	D3D12_STATE_OBJECT_DESC Desc;
	Desc.NumSubobjects = (uint32)SubObjects.size();
	Desc.pSubobjects = SubObjects.data();
	Desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
	
	#ifdef TIX_DEBUG
		PrintStateObjectDesc(&Desc);
	#endif
	VALIDATE_HRESULT(DXRDevice->CreateStateObject(&Desc, IID_PPV_ARGS(&RTXStateObject)));
	RTXStateObject->SetName(L"RTXStateObject");
}
void TRTXTest2::BuildGeometryTV()
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

void TRTXTest2::LoadMeshBufferExpand()
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

	// Expand vertex buffer
	TVector<vector3df> VBExpand;
	VBExpand.resize(MBIndexCount);
	TI_ASSERT(MeshHeader->IndexType == EIT_32BIT);
	const uint32* IndexDataSrc = (const uint32*)IndexDataStart;
	for (uint32 i = 0; i < MBIndexCount; i++)
	{
		uint32 Index = IndexDataSrc[i];
		const int8* VertexData = VertexDataStart + Index * VertexStride;
		VBExpand[i] = *(const vector3df*)VertexData;
	}

	// Create Vertex Buffer And Index Buffer
	// Create the vertex buffer resource in the GPU's default heap and copy vertex data into it using the upload heap.
	// The upload resource must not be released until after the GPU has finished using it.

	const int32 BufferSize = MeshHeader->PrimitiveCount * 3 * sizeof(vector3df);
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
		VertexData.pData = reinterpret_cast<const uint8*>(VBExpand.data());
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
}

// Build acceleration structures needed for raytracing.
void TRTXTest2::BuildAccelerationStructures()
{
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Triangles.IndexBuffer = IndexBuffer->GetGPUVirtualAddress();
	geometryDesc.Triangles.IndexCount = static_cast<UINT>(IndexBuffer->GetDesc().Width) / sizeof(Index);
	geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometryDesc.Triangles.VertexCount = static_cast<UINT>(VertexBuffer->GetDesc().Width) / sizeof(Vertex);
	geometryDesc.Triangles.VertexBuffer.StartAddress = VertexBuffer->GetGPUVirtualAddress();
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);

	// Mark the geometry as opaque. 
	// PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
	// Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	// Get required sizes for an acceleration structure.
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = buildFlags;
	topLevelInputs.NumDescs = 1;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	DXRDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
	TI_ASSERT(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = topLevelInputs;
	bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	bottomLevelInputs.pGeometryDescs = &geometryDesc;
	DXRDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
	TI_ASSERT(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

	//AllocateUAVBuffer(device, max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes), &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");
	{
		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&BLASScratch)));
		if (BLASScratch)
		{
			BLASScratch->SetName(L"scratchResource");
		}
	}

	// Allocate resources for acceleration structures.
	// Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
	// Default heap is OK since the application doesn’t need CPU read/write access to them. 
	// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
	// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
	//  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
	//  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
	{
		D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

		//AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &BLASRes, initialResourceState, L"BottomLevelAccelerationStructure");
		{
			auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
				&uploadHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				initialResourceState,
				nullptr,
				IID_PPV_ARGS(&BLASRes)));
			if (BLASRes)
			{
				BLASRes->SetName(L"BottomLevelAccelerationStructure");
			}
		}
		//AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &TLASRes, initialResourceState, L"TopLevelAccelerationStructure");
		{
			auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(topLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
				&uploadHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				initialResourceState,
				nullptr,
				IID_PPV_ARGS(&TLASRes)));
			if (TLASRes)
			{
				TLASRes->SetName(L"TopLevelAccelerationStructure");
			}
		}
	}

	// Create an instance desc for the bottom-level acceleration structure.
	D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
	instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
	instanceDesc.InstanceMask = 1;
	instanceDesc.AccelerationStructure = BLASRes->GetGPUVirtualAddress();
	//AllocateUploadBuffer(device, &instanceDesc, sizeof(instanceDesc), &instanceDescs, L"InstanceDescs");
	{
		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(instanceDesc));
		VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&TLASInstance)));
		if (TLASInstance)
		{
			TLASInstance->SetName(L"TLASInstance");
		}
		void* pMappedData;
		TLASInstance->Map(0, nullptr, &pMappedData);
		memcpy(pMappedData, &instanceDesc, sizeof(instanceDesc));
		TLASInstance->Unmap(0, nullptr);
	}

	// Bottom Level Acceleration Structure desc
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
	{
		bottomLevelBuildDesc.Inputs = bottomLevelInputs;
		bottomLevelBuildDesc.ScratchAccelerationStructureData = BLASScratch->GetGPUVirtualAddress();
		bottomLevelBuildDesc.DestAccelerationStructureData = BLASRes->GetGPUVirtualAddress();
	}

	// Top Level Acceleration Structure desc
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
	{
		topLevelInputs.InstanceDescs = TLASInstance->GetGPUVirtualAddress();
		topLevelBuildDesc.Inputs = topLevelInputs;
		topLevelBuildDesc.DestAccelerationStructureData = TLASRes->GetGPUVirtualAddress();
		topLevelBuildDesc.ScratchAccelerationStructureData = BLASScratch->GetGPUVirtualAddress();
	}

	auto BuildAccelerationStructure = [&](auto* raytracingCommandList)
	{
		raytracingCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
		CD3DX12_RESOURCE_BARRIER UAV = CD3DX12_RESOURCE_BARRIER::UAV(BLASRes.Get());
		raytracingCommandList->ResourceBarrier(1, &UAV);
		raytracingCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
	};

	// Build acceleration structure.
	BuildAccelerationStructure(DXRCommandList.Get());

	// Kick off acceleration structure construction.
	//m_deviceResources->ExecuteCommandList();

	// Wait for GPU to finish as the locally created temporary GPU resources will get released once we go out of scope.
	//m_deviceResources->WaitForGpu();
}

void TRTXTest2::BuildBLAS()
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
}

void TRTXTest2::BuildTLAS()
{
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

void TRTXTest2::BuildShaderTablesSample()
{
	void* rayGenShaderIdentifier;
	void* missShaderIdentifier;
	void* hitGroupShaderIdentifier;
	auto GetShaderIdentifiers = [&](auto* stateObjectProperties)
	{
		rayGenShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_raygenShaderName);
		missShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_missShaderName);
		hitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_hitGroupName);
	};

	// Get shader identifiers.
	UINT shaderIdentifierSize;
	{
		ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
		VALIDATE_HRESULT(RTXStateObject.As(&stateObjectProperties));
		GetShaderIdentifiers(stateObjectProperties.Get());
		shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	}

	// Ray gen shader table
	{
		struct RootArguments {
			RayGenConstantBuffer cb;
		} rootArguments;
		rootArguments.cb = m_rayGenCB;

		UINT numShaderRecords = 1;
		UINT shaderRecordSize = shaderIdentifierSize + sizeof(rootArguments);

		ShaderTable rayGenShaderTable(D3dDevice.Get(), numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
		rayGenShaderTable.push_back(ShaderRecord(rayGenShaderIdentifier, shaderIdentifierSize, &rootArguments, sizeof(rootArguments)));
		m_rayGenShaderTable = rayGenShaderTable.GetResource();
	}

	// Miss shader table
	{
		UINT numShaderRecords = 1;
		UINT shaderRecordSize = shaderIdentifierSize;
		ShaderTable missShaderTable(D3dDevice.Get(), numShaderRecords, shaderRecordSize, L"MissShaderTable");
		missShaderTable.push_back(ShaderRecord(missShaderIdentifier, shaderIdentifierSize));
		m_missShaderTable = missShaderTable.GetResource();
	}

	// Hit group shader table
	{
		UINT numShaderRecords = 1;
		UINT shaderRecordSize = shaderIdentifierSize;
		ShaderTable hitGroupShaderTable(D3dDevice.Get(), numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");
		hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderIdentifier, shaderIdentifierSize));
		m_hitGroupShaderTable = hitGroupShaderTable.GetResource();
	}

	// Test put all content in one table
	{
		struct RootArguments {
			RayGenConstantBuffer cb;
		} rootArguments;
		rootArguments.cb = m_rayGenCB;

		UINT numShaderRecords = 1;
		UINT shaderRecordSize = shaderIdentifierSize + sizeof(rootArguments);

		int32 ShaderTableSize = 0;
		RayGenShaderOffsetAndSize.X = ShaderTableSize;
		RayGenShaderOffsetAndSize.Y = TMath::Align(shaderRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		ShaderTableSize += TMath::Align(RayGenShaderOffsetAndSize.Y, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		// Miss
		MissShaderOffsetAndSize.X = ShaderTableSize;
		MissShaderOffsetAndSize.Y = TMath::Align(shaderIdentifierSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		ShaderTableSize += TMath::Align(MissShaderOffsetAndSize.Y, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		// Hit Group
		HitGroupOffsetAndSize.X = ShaderTableSize;
		HitGroupOffsetAndSize.Y = TMath::Align(shaderIdentifierSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		ShaderTableSize += TMath::Align(HitGroupOffsetAndSize.Y, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		TI_ASSERT(ShaderTableRes == nullptr);
		CD3DX12_RESOURCE_DESC ConstantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(ShaderTableSize);
		CD3DX12_HEAP_PROPERTIES UploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		VALIDATE_HRESULT(DXRDevice->CreateCommittedResource(
			&UploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&ConstantBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&ShaderTableRes)));
		ShaderTableRes->SetName(L"ShaderTableRes");
		// Build shader table data
		uint8* Data = ti_new uint8[ShaderTableSize];
		memset(Data, 0, ShaderTableSize);
		int32 Offset = 0;
		memcpy(Data + Offset, rayGenShaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		memcpy(Data + Offset + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, &rootArguments, sizeof(RootArguments));
		Offset += TMath::Align(RayGenShaderOffsetAndSize.Y, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		memcpy(Data + Offset, missShaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		Offset += TMath::Align(MissShaderOffsetAndSize.Y, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		memcpy(Data + Offset, hitGroupShaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		Offset += TMath::Align(HitGroupOffsetAndSize.Y, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		// Map the constant buffers.
		uint8* MappedConstantBuffer = nullptr;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		VALIDATE_HRESULT(ShaderTableRes->Map(0, &readRange, reinterpret_cast<void**>(&MappedConstantBuffer)));
		memcpy(MappedConstantBuffer, Data, ShaderTableSize);
		ShaderTableRes->Unmap(0, nullptr);
		ti_delete[] Data;
	}
}

// Create 2D output texture for raytracing.
void TRTXTest2::CreateRaytracingOutputResource()
{
	// Create the output resource. The dimensions and format should match the swap-chain.
	auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(OutputTextureFormat, WinWidth, WinHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	VALIDATE_HRESULT(D3dDevice->CreateCommittedResource(
		&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_raytracingOutput)));
	m_raytracingOutput->SetName(L"m_raytracingOutput");

	D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle = DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetCPUDescriptorHandleForHeapStart();
	m_raytracingOutputResourceUAVDescriptorHeapIndex = 0;
	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	D3dDevice->CreateUnorderedAccessView(m_raytracingOutput.Get(), nullptr, &UAVDesc, uavDescriptorHandle);
	m_raytracingOutputResourceUAVGpuDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetGPUDescriptorHandleForHeapStart(), m_raytracingOutputResourceUAVDescriptorHeapIndex, DescriptorIncSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);
}

void TRTXTest2::BuildBLASExpand()
{
	// no index buffer, only expanded vertex buffer
	// ================= BLAS =================
	{
		// Create Geometry Desc
		D3D12_RAYTRACING_GEOMETRY_DESC GeometryDesc = {};
		GeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		GeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		GeometryDesc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;
		GeometryDesc.Triangles.IndexBuffer = 0;
		GeometryDesc.Triangles.IndexCount = 0;

		GeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;	// Position always be RGB32F
		GeometryDesc.Triangles.VertexBuffer.StartAddress = VertexBuffer->GetGPUVirtualAddress();
		GeometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(vector3df);
		GeometryDesc.Triangles.VertexCount = MBIndexCount;

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
}

void TRTXTest2::BuildShaderTables()
{
	// Build Shader Table
	{
		ComPtr<ID3D12StateObjectProperties> StateObjectProperties;
		//VALIDATE_HRESULT(RTXStateObject.As(&StateObjectProperties));
		VALIDATE_HRESULT(RTXStateObject->QueryInterface(IID_PPV_ARGS(&StateObjectProperties)));

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

		TI_ASSERT(ShaderTableRes == nullptr);
		CD3DX12_RESOURCE_DESC ConstantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(ShaderTableSize);
		CD3DX12_HEAP_PROPERTIES UploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		VALIDATE_HRESULT(DXRDevice->CreateCommittedResource(
			&UploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&ConstantBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&ShaderTableRes)));
		ShaderTableRes->SetName(L"ShaderTableRes");
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
		VALIDATE_HRESULT(ShaderTableRes->Map(0, &readRange, reinterpret_cast<void**>(&MappedConstantBuffer)));
		memcpy(MappedConstantBuffer, Data, ShaderTableSize);
		ShaderTableRes->Unmap(0, nullptr);
		ti_delete[] Data;
	}
}

void TRTXTest2::Tick()
{
}

void TRTXTest2::Render()
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

void TRTXTest2::BeginFrame()
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

void TRTXTest2::EndFrame()
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
			VALIDATE_HRESULT(GraphicsFence->SetEventOnCompletion(FenceValues[CurrentFrame], FenceEvent.Get()));
			WaitForSingleObjectEx(FenceEvent.Get(), INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		FenceValues[CurrentFrame] = currentFenceValue + 1;
	}
}

void TRTXTest2::WaitForGPU()
{
	if (GraphicsCommandQueue != nullptr && GraphicsFence && FenceEvent.IsValid())
	{
		// Schedule a Signal command in the GPU queue.
		UINT64 fenceValue = FenceValues[CurrentFrame];
		if (SUCCEEDED(GraphicsCommandQueue->Signal(GraphicsFence.Get(), fenceValue)))
		{
			// Wait until the Signal has been processed.
			if (SUCCEEDED(GraphicsFence->SetEventOnCompletion(fenceValue, FenceEvent.Get())))
			{
				WaitForSingleObjectEx(FenceEvent.Get(), INFINITE, FALSE);

				// Increment the fence value for the current frame.
				FenceValues[CurrentFrame]++;
			}
		}
	}
}

void TRTXTest2::UpdateCamInfo()
{
}

void TRTXTest2::DispatchRays()
{
	auto DispatchRays = [&](auto* commandList, auto* stateObject, auto* dispatchDesc)
	{
		// Since each shader table has only one shader record, the stride is same as the size.
		//dispatchDesc->HitGroupTable.StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress();
		//dispatchDesc->HitGroupTable.SizeInBytes = m_hitGroupShaderTable->GetDesc().Width;
		//dispatchDesc->HitGroupTable.StrideInBytes = dispatchDesc->HitGroupTable.SizeInBytes;
		//dispatchDesc->MissShaderTable.StartAddress = m_missShaderTable->GetGPUVirtualAddress();
		//dispatchDesc->MissShaderTable.SizeInBytes = m_missShaderTable->GetDesc().Width;
		//dispatchDesc->MissShaderTable.StrideInBytes = dispatchDesc->MissShaderTable.SizeInBytes;
		//dispatchDesc->RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable->GetGPUVirtualAddress();
		//dispatchDesc->RayGenerationShaderRecord.SizeInBytes = m_rayGenShaderTable->GetDesc().Width;


		// RayGen is the first entry in the shader-table
		dispatchDesc->RayGenerationShaderRecord.StartAddress =
			ShaderTableRes.Get()->GetGPUVirtualAddress() + RayGenShaderOffsetAndSize.X;
		dispatchDesc->RayGenerationShaderRecord.SizeInBytes = RayGenShaderOffsetAndSize.Y;

		// Miss is the second entry in the shader-table
		dispatchDesc->MissShaderTable.StartAddress =
			ShaderTableRes.Get()->GetGPUVirtualAddress() + MissShaderOffsetAndSize.X;
		dispatchDesc->MissShaderTable.StrideInBytes = MissShaderOffsetAndSize.Y;
		dispatchDesc->MissShaderTable.SizeInBytes = MissShaderOffsetAndSize.Y;   // Only a s single miss-entry

		// Hit is the third entry in the shader-table
		dispatchDesc->HitGroupTable.StartAddress =
			ShaderTableRes.Get()->GetGPUVirtualAddress() + HitGroupOffsetAndSize.X;
		dispatchDesc->HitGroupTable.StrideInBytes = HitGroupOffsetAndSize.Y;
		dispatchDesc->HitGroupTable.SizeInBytes = HitGroupOffsetAndSize.Y;

		dispatchDesc->Width = WinWidth;
		dispatchDesc->Height = WinHeight;
		dispatchDesc->Depth = 1;
		commandList->SetPipelineState1(stateObject);
		commandList->DispatchRays(dispatchDesc);
	};

	DXRCommandList->SetComputeRootSignature(GlobalRS.Get());

	// Bind the heaps, acceleration structure and dispatch rays.    
	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
	//DXRCommandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
	DXRCommandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::OutputViewSlot, m_raytracingOutputResourceUAVGpuDescriptor);
	DXRCommandList->SetComputeRootShaderResourceView(GlobalRootSignatureParams::AccelerationStructureSlot, TLASRes->GetGPUVirtualAddress());
	DispatchRays(DXRCommandList.Get(), RTXStateObject.Get(), &dispatchDesc);
	return;
	/////////////////////////////////////////////////////////////////////



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
		ShaderTableRes.Get()->GetGPUVirtualAddress() + RayGenShaderOffsetAndSize.X;
	RaytraceDesc.RayGenerationShaderRecord.SizeInBytes = RayGenShaderOffsetAndSize.Y;

	// Miss is the second entry in the shader-table
	RaytraceDesc.MissShaderTable.StartAddress =
		ShaderTableRes.Get()->GetGPUVirtualAddress() + MissShaderOffsetAndSize.X;
	RaytraceDesc.MissShaderTable.StrideInBytes = MissShaderOffsetAndSize.Y;
	RaytraceDesc.MissShaderTable.SizeInBytes = MissShaderOffsetAndSize.Y;   // Only a s single miss-entry

	// Hit is the third entry in the shader-table
	RaytraceDesc.HitGroupTable.StartAddress =
		ShaderTableRes.Get()->GetGPUVirtualAddress() + HitGroupOffsetAndSize.X;
	RaytraceDesc.HitGroupTable.StrideInBytes = HitGroupOffsetAndSize.Y;
	RaytraceDesc.HitGroupTable.SizeInBytes = HitGroupOffsetAndSize.Y;

	// Dispatch
	DXRCommandList->DispatchRays(&RaytraceDesc);
	//RHI->TraceRays(RtxPSO, TraceSize);
}

// Copy the raytracing output to the backbuffer.
void TRTXTest2::CopyRaytracingOutputToBackbuffer()
{
	ID3D12Resource* RenderTarget = BackBufferRTs[CurrentFrame].Get();

	D3D12_RESOURCE_BARRIER preCopyBarriers[2];
	preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RenderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
	preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_raytracingOutput.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	GraphicsCommandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

	GraphicsCommandList->CopyResource(RenderTarget, m_raytracingOutput.Get());

	D3D12_RESOURCE_BARRIER postCopyBarriers[2];
	postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RenderTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
	postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_raytracingOutput.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	GraphicsCommandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
	return;


	//ID3D12Resource* RenderTarget = BackBufferRTs[CurrentFrame].Get();

	//D3D12_RESOURCE_BARRIER preCopyBarriers[2];
	//preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RenderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
	//preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(OutputTexture.Get(), OutputTextureState, D3D12_RESOURCE_STATE_COPY_SOURCE);
	//DXRCommandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);
	//OutputTextureState = D3D12_RESOURCE_STATE_COPY_SOURCE;

	//DXRCommandList->CopyResource(RenderTarget, OutputTexture.Get());

	//D3D12_RESOURCE_BARRIER postCopyBarriers[1];
	//postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(RenderTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
	//DXRCommandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}