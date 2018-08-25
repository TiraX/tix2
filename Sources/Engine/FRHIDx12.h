/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

#include "dx12/d3dx12.h"

using namespace Microsoft::WRL;

namespace tix
{
	enum E_HEAP_TYPE
	{
		EHT_CBV_SRV_UAV,
		EHT_SAMPLER,
		EHT_RTV,
		EHT_DSV,

		EHT_COUNT,
	};

	class FDescriptorHeapDx12
	{
	public:
		ComPtr<ID3D12DescriptorHeap> DescriptorHeap;
		uint32 DescriptorIncSize;
		TVector<uint32> AvaibleDescriptorHeapSlots;
		uint32 DescriptorAllocated;

		FDescriptorHeapDx12()
			: DescriptorIncSize(0)
			, DescriptorAllocated(0)
		{}
	};

	class FFrameResourcesDx12;
	// Render hardware interface use DirectX 12
	class FRHIDx12 : public FRHI
	{
	public:
		virtual ~FRHIDx12();

		// RHI common methods
		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual void WaitingForGpu() override;

		virtual FTexturePtr CreateTexture() override;
		virtual FTexturePtr CreateTexture(E_PIXEL_FORMAT Format, int32 Width, int32 Height) override;
		virtual FMeshBufferPtr CreateMeshBuffer() override;
		virtual FPipelinePtr CreatePipeline() override;
		virtual FUniformBufferPtr CreateUniformBuffer() override;
		virtual FRenderTargetPtr CreateRenderTarget() override;

		virtual bool UpdateHardwareResource(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData) override;
		virtual bool UpdateHardwareResource(FTexturePtr Texture, TTexturePtr InTexData) override;
		virtual bool UpdateHardwareResource(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc) override;
		virtual bool UpdateHardwareResource(FUniformBufferPtr UniformBuffer, void* InData, int32 InDataSize) override;
		virtual bool UpdateHardwareResource(FRenderTargetPtr RenderTarget, TRenderTargetPtr InRenderTarget) override;

		virtual void SetMeshBuffer(FMeshBufferPtr InMeshBuffer) override;
		virtual void SetPipeline(FPipelinePtr InPipeline) override;
		virtual void SetUniformBuffer(FUniformBufferPtr InUniformBuffer) override;

		virtual void DrawPrimitiveIndexedInstanced(
			uint32 IndexCountPerInstance,
			uint32 InstanceCount,
			uint32 StartIndexLocation,
			int32 BaseVertexLocation,
			uint32 StartInstanceLocation) override;

		// DirectX 12 specified methods
		void RecallDescriptor(uint32 HeapIndex);

	protected: 
		FRHIDx12();

		void Init();

	private:
		void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);
		void CreateWindowsSizeDependentResources();
		void MoveToNextFrame();
		void HoldResourceReference(FRenderResourcePtr InResource);
		void HoldResourceReference(ComPtr<ID3D12Resource> InDxResource);

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
			_In_reads_(NumSubresources) const D3D12_SUBRESOURCE_DATA* pSrcData);
		uint64 UpdateSubresources(
			_In_ ID3D12GraphicsCommandList* pCmdList,
			_In_ ID3D12Resource* pDestinationResource,
			_In_ ID3D12Resource* pIntermediate,
			uint64 IntermediateOffset,
			_In_range_(0, D3D12_REQ_SUBRESOURCES) uint32 FirstSubresource,
			_In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) uint32 NumSubresources,
			_In_reads_(NumSubresources) D3D12_SUBRESOURCE_DATA* pSrcData);

		void Transition(
			_In_ ID3D12Resource* pResource,
			D3D12_RESOURCE_STATES stateBefore,
			D3D12_RESOURCE_STATES stateAfter,
			uint32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

		void FlushResourceBarriers(
			_In_ ID3D12GraphicsCommandList* pCmdList);

		uint32 AllocateDescriptorSlot();
		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(uint32 Index);
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(uint32 Index);

	private:
		ComPtr<ID3D12Device> D3dDevice;
		ComPtr<IDXGIFactory4> DxgiFactory;
		ComPtr<IDXGISwapChain3> SwapChain;
		ComPtr<ID3D12Resource> BackBufferRTs[FRHIConfig::FrameBufferNum];
		D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptors[FRHIConfig::FrameBufferNum];
		ComPtr<ID3D12Resource> DepthStencil;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilDescriptor;
		ComPtr<ID3D12DescriptorHeap> RtvHeap;
		ComPtr<ID3D12DescriptorHeap> DsvHeap;
		ComPtr<ID3D12CommandQueue> CommandQueue;
		ComPtr<ID3D12CommandAllocator> CommandAllocators[FRHIConfig::FrameBufferNum];

		ComPtr<ID3D12PipelineState> DefaultPipelineState;
		ComPtr<ID3D12GraphicsCommandList> CommandList;

		ComPtr<ID3D12RootSignature> RootSignature;

		// CPU/GPU Synchronization.
		ComPtr<ID3D12Fence> Fence;
		uint64 FenceValues[FRHIConfig::FrameBufferNum];
		HANDLE FenceEvent;
		uint32 CurrentFrame;

		uint32 RtvDescriptorSize;

		// Barriers
		static const uint32 MaxResourceBarrierBuffers = 16;
		D3D12_RESOURCE_BARRIER ResourceBarrierBuffers[MaxResourceBarrierBuffers];
		uint32 NumBarriersToFlush;

		// Descriptor heaps
		ComPtr<ID3D12DescriptorHeap> DescriptorHeap;
		uint32 DescriptorIncSize;
		TVector<uint32> AvaibleDescriptorHeapSlots;
		uint32 DescriptorAllocated;

		// Frame on the fly Resource holders
		FFrameResourcesDx12 * ResHolders[FRHIConfig::FrameBufferNum];

		friend class FRHI;
	};
}
#endif	// COMPILE_WITH_RHI_DX12