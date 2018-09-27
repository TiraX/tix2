/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

#include "dx12/d3dx12.h"
#include "FRHIDescriptorHeapDx12.h"
#include "FRootSignatureDx12.h"

using namespace Microsoft::WRL;

namespace tix
{
	class FFrameResourcesDx12;
	class FGPUResourceDx12;
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
		virtual FTexturePtr CreateTexture(const TTextureDesc& Desc) override;
		virtual FMeshBufferPtr CreateMeshBuffer() override;
		virtual FPipelinePtr CreatePipeline() override;
		virtual FUniformBufferPtr CreateUniformBuffer(uint32 UBFlag) override;
		virtual FRenderTargetPtr CreateRenderTarget(int32 W, int32 H) override;

		virtual bool UpdateHardwareResource(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData) override;
		virtual bool UpdateHardwareResource(FTexturePtr Texture, TTexturePtr InTexData) override;
		virtual bool UpdateHardwareResource(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc) override;
		virtual bool UpdateHardwareResource(FUniformBufferPtr UniformBuffer, void* InData, int32 InDataSize, uint32 UBFlag) override;
		virtual bool UpdateHardwareResource(FRenderTargetPtr RenderTarget) override;

		virtual void SetMeshBuffer(FMeshBufferPtr InMeshBuffer) override;
		virtual void SetPipeline(FPipelinePtr InPipeline) override;
		virtual void SetUniformBuffer(int32 BindIndex, FUniformBufferPtr InUniformBuffer) override;
		virtual void SetShaderTexture(int32 BindIndex, FTexturePtr InTexture) override;

		virtual void DrawPrimitiveIndexedInstanced(
			uint32 IndexCountPerInstance,
			uint32 InstanceCount,
			uint32 StartIndexLocation,
			int32 BaseVertexLocation,
			uint32 StartInstanceLocation) override;

		virtual void SetViewport(const FViewport& InViewport);
		virtual void PushRenderTarget(FRenderTargetPtr RT);
		virtual FRenderTargetPtr PopRenderTarget();

		void RecallDescriptor(E_HEAP_TYPE HeapType, uint32 DescriptorIndex, E_UNIFORMBUFFER_SECTION UniformBufferSection = UB_SECTION_NORMAL);
		void RecallDescriptor(E_HEAP_TYPE HeapType, D3D12_CPU_DESCRIPTOR_HANDLE Descriptor, E_UNIFORMBUFFER_SECTION UniformBufferSection = UB_SECTION_NORMAL);

	protected: 
		FRHIDx12();

		void Init();

	private:
		void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);
		void CreateWindowsSizeDependentResources();
		void MoveToNextFrame();
		void HoldResourceReference(FRenderResourcePtr InResource);
		void HoldResourceReference(ComPtr<ID3D12Resource> InDxResource);

		void SetResourceName(ID3D12Resource* InDxResource, const TString& InName);
		void SetResourceName(ID3D12PipelineState* InDxResource, const TString& InName);

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
			FGPUResourceDx12* GPUResource,
			D3D12_RESOURCE_STATES stateAfter,
			uint32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

		void Transition(
			_In_ ID3D12Resource* pResource,
			D3D12_RESOURCE_STATES stateBefore,
			D3D12_RESOURCE_STATES stateAfter,
			uint32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

		void FlushResourceBarriers(
			_In_ ID3D12GraphicsCommandList* pCmdList);

		void SetRenderTarget(FRenderTargetPtr RT);
		
	private:
		ComPtr<ID3D12Device> D3dDevice;
		ComPtr<IDXGIFactory4> DxgiFactory;
		ComPtr<IDXGISwapChain3> SwapChain;
		
		// Back buffers and Depth Stencil buffers
		ComPtr<ID3D12Resource> BackBufferRTs[FRHIConfig::FrameBufferNum];
		uint32 BackBufferDescriptorIndex[FRHIConfig::FrameBufferNum];
		D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptors[FRHIConfig::FrameBufferNum];
		ComPtr<ID3D12Resource> DepthStencil;
		uint32 DepthStencilDescriptorIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilDescriptor;

		// Commands
		ComPtr<ID3D12CommandQueue> CommandQueue;
		ComPtr<ID3D12CommandAllocator> CommandAllocators[FRHIConfig::FrameBufferNum];
		ComPtr<ID3D12GraphicsCommandList> CommandList;

		// RootSignature
		FRootSignatureDx12 RootSignature;

		// Descriptor heaps
		FDescriptorHeapDx12 DescriptorHeaps[EHT_COUNT];

		// CPU/GPU Synchronization.
		ComPtr<ID3D12Fence> Fence;
		uint64 FenceValues[FRHIConfig::FrameBufferNum];
		HANDLE FenceEvent;
		uint32 CurrentFrame;

		// Barriers
		static const uint32 MaxResourceBarrierBuffers = 16;
		D3D12_RESOURCE_BARRIER ResourceBarrierBuffers[MaxResourceBarrierBuffers];
		uint32 NumBarriersToFlush;

		// Frame on the fly Resource holders
		FFrameResourcesDx12 * ResHolders[FRHIConfig::FrameBufferNum];

		friend class FRHI;
	};
}
#endif	// COMPILE_WITH_RHI_DX12