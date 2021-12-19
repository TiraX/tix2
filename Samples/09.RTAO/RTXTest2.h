/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include <dxgi1_6.h>
#include <d3d12.h>
#include "Dx12/d3dx12.h"
using namespace Microsoft::WRL;

namespace GlobalRootSignatureParams {
	enum Value {
		OutputViewSlot = 0,
		AccelerationStructureSlot,
		Count
	};
}

namespace LocalRootSignatureParams {
	enum Value {
		ViewportConstantSlot = 0,
		Count
	};
}

struct Viewport
{
	float left;
	float top;
	float right;
	float bottom;
};

struct RayGenConstantBuffer
{
	Viewport viewport;
	Viewport stencil;
};

class TRTXTest2
{
public:
	TRTXTest2();
	~TRTXTest2();

	void Run();

protected:
	void Init();
	void CreateAWindow();
	bool IsRunning();
	void InitD3D12();

	void CreateResources();
	void InitCamera();
	void CreateOutputTexture();
	void CreateRaytracingPipelineObject();
	void BuildGeometryTV();
	void LoadMeshBufferExpand();
	void CreateShaderParameters();
	void BuildBLAS();
	void BuildBLASExpand();
	void BuildTLAS();
	void BuildShaderTables();

	void CreateRootSignatures();

	void Tick();
	void Render();
	void UpdateCamInfo();
	void DispatchRays();

	void BeginFrame();
	void EndFrame();
	void WaitForGPU();

	void CopyRaytracingOutputToBackbuffer();
	void BuildGeometry();
	void BuildAccelerationStructures();
	void BuildShaderTablesSample();
	void CreateRaytracingOutputResource();
private:
	bool Inited;
	HWND HWnd;

	ComPtr<ID3D12Device> D3dDevice;
	ComPtr<IDXGIFactory4> DxgiFactory;
	ComPtr<IDXGISwapChain3> SwapChain;

	// Back buffers and Depth Stencil buffers
	ComPtr<ID3D12Resource> BackBufferRTs[FRHIConfig::FrameBufferNum];
	D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptors[FRHIConfig::FrameBufferNum];
	ComPtr<ID3D12Resource> DepthStencil;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilDescriptor;

	// Commands
	ComPtr<ID3D12CommandQueue> GraphicsCommandQueue;

	ComPtr<ID3D12CommandAllocator> GraphicsAllocators[FRHIConfig::FrameBufferNum];
	ComPtr<ID3D12GraphicsCommandList> GraphicsCommandList;
	ComPtr<ID3D12Fence> GraphicsFence;

	// Descriptor heaps
	ComPtr<ID3D12DescriptorHeap> DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	uint32 DescriptorIncSize[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	// CPU/GPU Synchronization.
	uint64 FenceValues[FRHIConfig::FrameBufferNum];
	Wrappers::Event FenceEvent;
	uint32 CurrentFrame;

	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;

	// Barriers
	//static const uint32 MaxResourceBarrierBuffers = 16;
	//D3D12_RESOURCE_BARRIER GraphicsBarrierBuffers[MaxResourceBarrierBuffers];
	//uint32 GraphicsNumBarriersToFlush;
	//D3D12_RESOURCE_BARRIER ComputeBarrierBuffers[MaxResourceBarrierBuffers];
	//uint32 ComputeNumBarriersToFlush;

	ComPtr<ID3D12Device5> DXRDevice;
	ComPtr<ID3D12GraphicsCommandList4> DXRCommandList;

	// Resources
	D3D12_RESOURCE_STATES OutputTextureState;
	ComPtr<ID3D12Resource> OutputTexture;
	ComPtr<ID3D12StateObject> RTXStateObject;
	ComPtr<ID3D12RootSignature> GlobalRS;
	ComPtr<ID3D12RootSignature> LocalRS;

	// Raytracing scene
	RayGenConstantBuffer m_rayGenCB;
	// Raytracing output
	ComPtr<ID3D12Resource> m_raytracingOutput;
	D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;
	UINT m_raytracingOutputResourceUAVDescriptorHeapIndex;

	// Mesh Buffer
	int32 MBVertexCount;
	int32 MBVertexStride;
	int32 MBIndexCount;
	ComPtr<ID3D12Resource> VertexBuffer;
	ComPtr<ID3D12Resource> VertexBufferUpload;
	ComPtr<ID3D12Resource> IndexBuffer;
	ComPtr<ID3D12Resource> IndexBufferUpload;

	// Acceleration Structure
	ComPtr<ID3D12Resource> BLASRes;
	ComPtr<ID3D12Resource> BLASScratch;
	ComPtr<ID3D12Resource> TLASRes;
	ComPtr<ID3D12Resource> TLASScratch;
	ComPtr<ID3D12Resource> TLASInstance;

	ComPtr<ID3D12Resource> m_missShaderTable;
	ComPtr<ID3D12Resource> m_hitGroupShaderTable;
	ComPtr<ID3D12Resource> m_rayGenShaderTable;

	// Dispatch parameters
	enum {
		INDEX_OUTPUT_TEXTURE = 0,
		INDEX_TLAS = 1,
	};
	vector2di RayGenShaderOffsetAndSize;
	vector2di MissShaderOffsetAndSize;
	vector2di HitGroupOffsetAndSize;
	ComPtr<ID3D12Resource> ShaderTableRes;
	int32 DispatchResourceTableStart;
	ComPtr<ID3D12Resource> ConstantBuffer;
	uint8* ConstantBufferMappedAddress;

	matrix4 MatProj;
	matrix4 MatView;
};