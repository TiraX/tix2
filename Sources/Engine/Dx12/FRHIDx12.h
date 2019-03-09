/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

#include "d3dx12.h"
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
		virtual void InitRHI() override;
		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual void WaitingForGpu() override;

		virtual FTexturePtr CreateTexture() override;
		virtual FTexturePtr CreateTexture(const TTextureDesc& Desc) override;
		virtual FUniformBufferPtr CreateUniformBuffer(uint32 InStructureSizeInBytes, uint32 Elements, uint32 Flag = 0) override;
		virtual FMeshBufferPtr CreateMeshBuffer() override;
		virtual FPipelinePtr CreatePipeline(FShaderPtr InShader) override;
		virtual FRenderTargetPtr CreateRenderTarget(int32 W, int32 H) override;
		virtual FShaderPtr CreateShader(const TShaderNames& InNames) override;
		virtual FShaderPtr CreateComputeShader(const TString& InComputeShaderName) override;
		virtual FArgumentBufferPtr CreateArgumentBuffer(FShaderPtr InShader) override;
		virtual FComputeTaskPtr CreateComputeTask(const TString& InName) override;

		virtual bool UpdateHardwareResource(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData) override;
		virtual bool UpdateHardwareResource(FTexturePtr Texture) override;
		virtual bool UpdateHardwareResource(FTexturePtr Texture, TTexturePtr InTexData) override;
		virtual bool UpdateHardwareResource(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc) override;
		virtual bool UpdateHardwareResource(FUniformBufferPtr UniformBuffer, void* InData) override;
		virtual bool UpdateHardwareResource(FRenderTargetPtr RenderTarget) override;
		virtual bool UpdateHardwareResource(FShaderPtr ShaderResource) override;
		virtual bool UpdateHardwareResource(FArgumentBufferPtr ArgumentBuffer, TStreamPtr ArgumentData, const TVector<FTexturePtr>& ArgumentTextures) override;

		virtual void PutConstantBufferInHeap(FUniformBufferPtr InUniformBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) override;
		virtual void PutTextureInHeap(FTexturePtr InTexture, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) override;
		virtual void PutBufferInHeap(FUniformBufferPtr InBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) override;
		virtual void PutRTColorInHeap(FTexturePtr InTexture, uint32 InHeapSlot) override;
		virtual void PutRTDepthInHeap(FTexturePtr InTexture, uint32 InHeapSlot) override;

		// Graphics
		virtual void SetGraphicsPipeline(FPipelinePtr InPipeline) override;
		virtual void SetMeshBuffer(FMeshBufferPtr InMeshBuffer) override;
		virtual void SetUniformBuffer(E_SHADER_STAGE ShaderStage, int32 BindIndex, FUniformBufferPtr InUniformBuffer) override;
		virtual void SetRenderResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable) override;
		virtual void SetShaderTexture(int32 BindIndex, FTexturePtr InTexture) override;
		virtual void SetArgumentBuffer(FArgumentBufferPtr InArgumentBuffer) override;

		virtual void SetStencilRef(uint32 InRefValue) override;
		virtual void DrawPrimitiveIndexedInstanced(FMeshBufferPtr MeshBuffer, uint32 InstanceCount) override;

		// Compute
		virtual void SetComputePipeline(FPipelinePtr InPipeline) override;
		virtual void SetComputeConstantBuffer(int32 BindIndex, FUniformBufferPtr InUniformBuffer) override;
		virtual void SetComputeResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable) override;

		virtual void DispatchCompute(uint32 GroupCountX, uint32 GroupCountY, uint32 GroupCountZ) override;

		virtual void SetViewport(const FViewport& InViewport) override;
		virtual void PushRenderTarget(FRenderTargetPtr RT, const int8* PassName = "UnnamedPass") override;
		virtual FRenderTargetPtr PopRenderTarget() override;

		ComPtr<ID3D12Device> GetD3dDevice()
		{
			return D3dDevice;
		}

		// For test only 
		TI_API ComPtr<ID3D12Device> GetD3DDevice()
		{
			return D3dDevice;
		}
		TI_API ComPtr<ID3D12GraphicsCommandList> GetRenderCommandList()
		{
			return RenderCommandList;
		}
		TI_API ComPtr<ID3D12GraphicsCommandList> GetComputeCommandList()
		{
			return ComputeCommandList;
		}
	protected: 
		FRHIDx12();

	private:
		void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);
		void CreateWindowsSizeDependentResources();
		void MoveToNextFrame();
		void HoldResourceReference(FRenderResourcePtr InResource);
		void HoldResourceReference(ComPtr<ID3D12Resource> InDxResource);

		void SetResourceName(ID3D12Resource* InDxResource, const TString& InName);
		void SetResourceName(ID3D12PipelineState* InDxResource, const TString& InName);
		FShaderBindingPtr CreateShaderBinding(const D3D12_ROOT_SIGNATURE_DESC& RSDesc);

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

		void FlushGraphicsBarriers(
			_In_ ID3D12GraphicsCommandList* pCmdList);

		void SetRenderTarget(FRenderTargetPtr RT);

		void InitRHIRenderResourceHeap(E_RENDER_RESOURCE_HEAP_TYPE Heap, uint32 HeapSize, uint32 HeapOffset);
		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(E_RENDER_RESOURCE_HEAP_TYPE Heap, uint32 SlotIndex);
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(E_RENDER_RESOURCE_HEAP_TYPE Heap, uint32 SlotIndex);
	private:
		ComPtr<ID3D12Device> D3dDevice;
		ComPtr<IDXGIFactory4> DxgiFactory;
		ComPtr<IDXGISwapChain3> SwapChain;
		
		// Back buffers and Depth Stencil buffers
		ComPtr<ID3D12Resource> BackBufferRTs[FRHIConfig::FrameBufferNum];
		FRenderResourceTablePtr BackBufferDescriptorTable;
		D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDescriptors[FRHIConfig::FrameBufferNum];
		ComPtr<ID3D12Resource> DepthStencil;
		FRenderResourceTablePtr DepthStencilDescriptorTable;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilDescriptor;

		// Commands
		ComPtr<ID3D12CommandQueue> RenderCommandQueue;
		ComPtr<ID3D12CommandAllocator> RenderCommandAllocators[FRHIConfig::FrameBufferNum];
		ComPtr<ID3D12GraphicsCommandList> RenderCommandList;
		// Compute Commands
		ComPtr<ID3D12CommandQueue> ComputeCommandQueue;
		ComPtr<ID3D12CommandAllocator> ComputeCommandAllocators[FRHIConfig::FrameBufferNum];
		ComPtr<ID3D12GraphicsCommandList> ComputeCommandList;

		// Descriptor heaps
		FDescriptorHeapDx12 DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		// CPU/GPU Synchronization.
		ComPtr<ID3D12Fence> RenderFence;
		ComPtr<ID3D12Fence> ComputeFence;
		uint64 FenceValues[FRHIConfig::FrameBufferNum];
		HANDLE FenceEvent;
		uint32 CurrentFrame;

		// Barriers
		static const uint32 MaxResourceBarrierBuffers = 16;
		D3D12_RESOURCE_BARRIER GraphicsBarrierBuffers[MaxResourceBarrierBuffers];
		uint32 GraphicsNumBarriersToFlush;
		D3D12_RESOURCE_BARRIER ComputeBarrierBuffers[MaxResourceBarrierBuffers];
		uint32 ComputeNumBarriersToFlush;

		// Frame on the fly Resource holders
		FFrameResourcesDx12 * ResHolders[FRHIConfig::FrameBufferNum];

		// Root Descriptor cache
		THMap<uint32, FShaderBindingPtr> ShaderBindingCache;

		friend class FRHI;
		friend class FDescriptorHeapDx12;
	};
}
#endif	// COMPILE_WITH_RHI_DX12