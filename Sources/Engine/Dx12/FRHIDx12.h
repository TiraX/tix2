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
		virtual void BeginRenderToFrameBuffer() override;

		virtual void BeginComputeTask(bool IsTileComputeShader = false) override;
		virtual void EndComputeTask(bool IsTileComputeShader = false) override;

		virtual void BeginEvent(const int8* InEventName) override;
		virtual void EndEvent() override;

		virtual int32 GetCurrentEncodingFrameIndex() override;
		virtual void WaitingForGpu() override;

		virtual FTexturePtr CreateTexture() override;
		virtual FTexturePtr CreateTexture(const TTextureDesc& Desc) override;
		virtual FUniformBufferPtr CreateUniformBuffer(uint32 InStructureSizeInBytes, uint32 Elements, uint32 Flag = 0) override;
		virtual FMeshBufferPtr CreateMeshBuffer() override;
		virtual FMeshBufferPtr CreateEmptyMeshBuffer(
			E_PRIMITIVE_TYPE InPrimType,
			uint32 InVSFormat,
			uint32 InVertexCount,
			E_INDEX_TYPE InIndexType,
			uint32 InIndexCount
		) override;
		virtual FInstanceBufferPtr CreateInstanceBuffer() override;
		virtual FInstanceBufferPtr CreateEmptyInstanceBuffer(uint32 InstanceCount, uint32 InstanceStride) override;
		virtual FPipelinePtr CreatePipeline(FShaderPtr InShader) override;
		virtual FRenderTargetPtr CreateRenderTarget(int32 W, int32 H) override;
		virtual FShaderPtr CreateShader(const TShaderNames& InNames) override;
		virtual FShaderPtr CreateComputeShader(const TString& InComputeShaderName) override;
		virtual FArgumentBufferPtr CreateArgumentBuffer(int32 ReservedSlots) override;
		virtual FGPUCommandSignaturePtr CreateGPUCommandSignature(FPipelinePtr Pipeline, const TVector<E_GPU_COMMAND_TYPE>& CommandStructure) override;
		virtual FGPUCommandBufferPtr CreateGPUCommandBuffer(FGPUCommandSignaturePtr GPUCommandSignature, uint32 CommandsCount, uint32 Flag = 0) override;

		virtual bool UpdateHardwareResourceMesh(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData) override;
		virtual bool UpdateHardwareResourceMesh(
			FMeshBufferPtr MeshBuffer, 
			uint32 VertexDataSize, 
			uint32 VertexDataStride, 
			uint32 IndexDataSize,
			E_INDEX_TYPE IndexType,
			const TString& BufferName) override;
		virtual bool UpdateHardwareResourceIB(FInstanceBufferPtr InstanceBuffer, TInstanceBufferPtr InInstanceData) override;
		virtual bool UpdateHardwareResourceTexture(FTexturePtr Texture) override;
		virtual bool UpdateHardwareResourceTexture(FTexturePtr Texture, TTexturePtr InTexData) override;
		virtual bool UpdateHardwareResourceTexture(FTexturePtr Texture, TImagePtr InTexData) override;
		virtual bool UpdateHardwareResourcePL(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc) override;
		virtual bool UpdateHardwareResourceTilePL(FPipelinePtr Pipeline, TTilePipelinePtr InTilePipelineDesc) override;
		virtual bool UpdateHardwareResourceUB(FUniformBufferPtr UniformBuffer, const void* InData) override;
		virtual bool UpdateHardwareResourceRT(FRenderTargetPtr RenderTarget) override;
		virtual bool UpdateHardwareResourceShader(FShaderPtr ShaderResource, TShaderPtr InShaderSource) override;
		virtual bool UpdateHardwareResourceAB(FArgumentBufferPtr ArgumentBuffer, FShaderPtr InShader, int32 SpecifiedBindingIndex = -1) override;
		virtual bool UpdateHardwareResourceGPUCommandSig(FGPUCommandSignaturePtr GPUCommandSignature) override;
		virtual bool UpdateHardwareResourceGPUCommandBuffer(FGPUCommandBufferPtr GPUCommandBuffer) override;
		virtual void PrepareDataForCPU(FUniformBufferPtr UniformBuffer) override;
		virtual bool CopyTextureRegion(FTexturePtr DstTexture, const recti& InDstRegion, uint32 DstMipLevel, FTexturePtr SrcTexture, uint32 SrcMipLevel) override;
		virtual bool CopyBufferRegion(FUniformBufferPtr DstBuffer, uint32 DstOffset, FUniformBufferPtr SrcBuffer, uint32 Length) override;
		virtual bool CopyBufferRegion(
			FMeshBufferPtr DstBuffer,
			uint32 DstVertexOffset,
			uint32 DstIndexOffset,
			FMeshBufferPtr SrcBuffer,
			uint32 SrcVertexOffset,
			uint32 VertexLength,
			uint32 SrcIndexOffset,
			uint32 IndexLength) override;
		virtual bool CopyBufferRegion(
			FInstanceBufferPtr DstBuffer,
			uint32 DstInstanceOffset,
			FInstanceBufferPtr SrcBuffer,
			uint32 SrcInstanceOffset,
			uint32 InstanceCount) override;

		virtual void PutConstantBufferInHeap(FUniformBufferPtr InUniformBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) override;
		virtual void PutTextureInHeap(FTexturePtr InTexture, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) override;
		virtual void PutRWTextureInHeap(FTexturePtr InTexture, uint32 InMipLevel, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) override;
		virtual void PutUniformBufferInHeap(FUniformBufferPtr InBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) override;
		virtual void PutInstanceBufferInHeap(FInstanceBufferPtr InBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) override;
		virtual void PutRTColorInHeap(FTexturePtr InTexture, uint32 InHeapSlot) override;
		virtual void PutRTDepthInHeap(FTexturePtr InTexture, uint32 InHeapSlot) override;

		// Graphics
		virtual void SetGraphicsPipeline(FPipelinePtr InPipeline) override;
		virtual void SetMeshBuffer(FMeshBufferPtr InMeshBuffer, FInstanceBufferPtr InInstanceBuffer) override;
		virtual void SetMeshBufferAtSlot(uint32 StartSlot, FMeshBufferPtr InMeshBuffer) override;
		virtual void SetInstanceBufferAtSlot(uint32 StartSlot, FInstanceBufferPtr InInstanceBuffer) override;
		virtual void SetUniformBuffer(E_SHADER_STAGE ShaderStage, int32 BindIndex, FUniformBufferPtr InUniformBuffer) override;
		virtual void SetRenderResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable) override;
		virtual void SetShaderTexture(int32 BindIndex, FTexturePtr InTexture) override;
		virtual void SetArgumentBuffer(int32 InBindIndex, FArgumentBufferPtr InArgumentBuffer) override;
		virtual void SetResourceStateUB(FUniformBufferPtr InUniformBuffer, E_RESOURCE_STATE NewState) override;
		virtual void SetResourceStateCB(FGPUCommandBufferPtr InCommandBuffer, E_RESOURCE_STATE NewState) override;

		virtual void SetStencilRef(uint32 InRefValue) override;
		virtual void DrawPrimitiveIndexedInstanced(FMeshBufferPtr MeshBuffer, uint32 InstanceCount, uint32 InstanceOffset) override;
		virtual void GraphicsCopyBuffer(FUniformBufferPtr Dest, uint32 DestOffset, FUniformBufferPtr Src, uint32 SrcOffset, uint32 CopySize) override;

		// Tile, For Metal, dx12 has empty implementation
		virtual void SetTilePipeline(FPipelinePtr InPipeline) override;
		virtual void SetTileBuffer(int32 BindIndex, FUniformBufferPtr InUniformBuffer) override;
		virtual void DispatchTile(const vector3di& GroupSize) override;

		// Compute
		virtual void SetComputePipeline(FPipelinePtr InPipeline) override;
		virtual void SetComputeBuffer(int32 BindIndex, FUniformBufferPtr InUniformBuffer) override;
		virtual void SetComputeResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable) override;
		virtual void SetComputeArgumentBuffer(int32 BindIndex, FArgumentBufferPtr InArgumentBuffer) override;
		virtual void SetComputeTexture(int32 BindIndex, FTexturePtr InTexture) override;

		virtual void DispatchCompute(const vector3di& GroupSize, const vector3di& GroupCount) override;
		virtual void ComputeCopyBuffer(FUniformBufferPtr Dest, uint32 DestOffset, FUniformBufferPtr Src, uint32 SrcOffset, uint32 CopySize) override;

		// GPU Command buffer
		virtual void ExecuteGPUCommands(FGPUCommandBufferPtr GPUCommandBuffer) override;

		virtual void SetViewport(const FViewport& InViewport) override;
		virtual void BeginRenderToRenderTarget(FRenderTargetPtr RT, uint32 MipLevel = 0, const int8* PassName = "UnnamedPass") override;

		ComPtr<ID3D12Device> GetD3dDevice()
		{
			return D3dDevice;
		}
	protected: 
		FRHIDx12();

	private:
		void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);
		void CreateWindowsSizeDependentResources();
		void MoveToNextFrame();
		void HoldResourceReference(FRenderResourcePtr InResource);
		void HoldResourceReference(ComPtr<ID3D12Resource> InDxResource);
		void InitGraphicsPipeline();

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

		void SetRenderTarget(FRenderTargetPtr RT, uint32 MipLevel);

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

		struct FCommandListDx12
		{
			ComPtr<ID3D12CommandAllocator> Allocators[FRHIConfig::FrameBufferNum];
			ComPtr<ID3D12GraphicsCommandList> CommandList;
			ComPtr<ID3D12Fence> Fence;

			FCommandListDx12()
			{
			}

			FCommandListDx12(const FCommandListDx12& Other)
			{
				*this = Other;
			}

			FCommandListDx12& operator = (const FCommandListDx12& Other)
			{
				for (int32 i = 0; i < FRHIConfig::FrameBufferNum; ++i)
				{
					Allocators[i] = Other.Allocators[i];
				}
				CommandList = Other.CommandList;
				Fence = Other.Fence;
				return *this;
			}

			void Create(ComPtr<ID3D12Device> Device, E_PIPELINE_TYPE PLType, int32 Index)
			{
				wchar_t Name[128];
				TWString AllocatorName, ListName;
				D3D12_COMMAND_LIST_TYPE ListType;
				if (PLType == EPL_GRAPHICS)
				{
					AllocatorName = L"GraphicsCommandAllocator";
					ListName = L"GraphicsCommandList";
					ListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
				}
				else
				{
					AllocatorName = L"ComputeCommandAllocator";
					ListName = L"ComputeCommandList";
					ListType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
				}
				for (uint32 n = 0; n < FRHIConfig::FrameBufferNum; n++)
				{
					swprintf_s(Name, 128, L"%s%d_%d", AllocatorName.c_str(), Index, n);
					VALIDATE_HRESULT(Device->CreateCommandAllocator(ListType, IID_PPV_ARGS(&Allocators[n])));
					Allocators[n]->SetName(Name);
				}
				swprintf_s(Name, 128, L"%s%d", ListName.c_str(), Index);
				VALIDATE_HRESULT(Device->CreateCommandList(
					0,
					ListType,
					Allocators[0].Get(),
					nullptr,
					IID_PPV_ARGS(&CommandList)));
				CommandList->SetName(Name);
				VALIDATE_HRESULT(CommandList->Close());

				VALIDATE_HRESULT(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
			}
		};

		// Commands
		ComPtr<ID3D12CommandQueue> GraphicsCommandQueue;
		TVector<FCommandListDx12> GraphicsCommandLists;
		FCommandListDx12 DefaultGraphicsCommandList;	// Default command list for render thread task render command execution

		// Compute Commands
		ComPtr<ID3D12CommandQueue> ComputeCommandQueue;
		TVector<FCommandListDx12> ComputeCommandLists;
		ComPtr<ID3D12Fence> FrameFence;

		ComPtr<ID3D12GraphicsCommandList> CurrentWorkingCommandList;

		// Descriptor heaps
		FDescriptorHeapDx12 DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		// CPU/GPU Synchronization.
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