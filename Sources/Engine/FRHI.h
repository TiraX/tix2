/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include "FRHIConfig.h"
#include "FViewport.h"
#include "FRenderResourceHeap.h"

namespace tix
{
	enum E_RHI_TYPE
	{
		ERHI_DX12 = 0,
		ERHI_METAL = 1,

		ERHI_NUM,
	};

	enum E_PIPELINE_TYPE
	{
		EPL_INVALID = -1,
		EPL_GRAPHICS,
		EPL_COMPUTE,
        EPL_BLIT,

		EPL_NUM,
	};

	enum E_RESOURCE_STATE
	{
		RESOURCE_STATE_COMMON,
		RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		RESOURCE_STATE_INDEX_BUFFER,
		RESOURCE_STATE_RENDER_TARGET,
		RESOURCE_STATE_UNORDERED_ACCESS,
		RESOURCE_STATE_DEPTH_WRITE,
		RESOURCE_STATE_DEPTH_READ,
		RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		RESOURCE_STATE_STREAM_OUT,
		RESOURCE_STATE_INDIRECT_ARGUMENT,
		RESOURCE_STATE_COPY_DEST,
		RESOURCE_STATE_COPY_SOURCE,
		RESOURCE_STATE_MESHBUFFER,	// For TiX to set VertexBuffer state and IndexBuffer state
		RESOURCE_STATE_RAYTRACING_AS,

		RESOURCE_STATE_NUM,
	};

	struct FBoundResource
	{
		FPipelinePtr Pipeline;
		FShaderBindingPtr ShaderBinding;

		void Reset()
		{
			Pipeline = nullptr;
			ShaderBinding = nullptr;
		}
	};

	class FFrameResources;
	// Render hardware interface
	class FRHI
	{
	public: 
		TI_API static FRHI* Get();
		static void CreateRHI();
		static void ReleaseRHI();

		static FRHIConfig RHIConfig;

		static uint32 GetGPUFrames()
		{
			return NumGPUFrames;
		}

		virtual void InitRHI() = 0;
		virtual void BeginFrame();
		virtual void EndFrame() = 0;
        virtual void BeginRenderToFrameBuffer() {};

		virtual void BeginComputeTask(bool IsTileComputeShader = false) = 0;
		virtual void EndComputeTask(bool IsTileComputeShader = false) = 0;

		virtual void BeginEvent(const int8* InEventName) = 0;
		virtual void BeginEvent(const int8* InEventName, int32 Index) = 0;
		virtual void EndEvent() = 0;

		virtual int32 GetCurrentEncodingFrameIndex() = 0;
		virtual void WaitingForGpu() = 0;

		// Create RTX related resources
		virtual FShaderPtr CreateRtxShaderLib(const TString& ShaderLibName) = 0;
		virtual FRtxPipelinePtr CreateRtxPipeline(FShaderPtr InShader) = 0;
		virtual FTopLevelAccelerationStructurePtr CreateTopLevelAccelerationStructure() = 0;
		virtual FBottomLevelAccelerationStructurePtr CreateBottomLevelAccelerationStructure() = 0;

		// Create Graphics and Compute related resources
		virtual FTexturePtr CreateTexture() = 0;
		virtual FTexturePtr CreateTexture(const TTextureDesc& Desc) = 0;
		virtual FUniformBufferPtr CreateUniformBuffer(uint32 InStructureSizeInBytes, uint32 Elements, uint32 Flag = 0) = 0;
		virtual FMeshBufferPtr CreateMeshBuffer() = 0;
		virtual FMeshBufferPtr CreateEmptyMeshBuffer(
			E_PRIMITIVE_TYPE InPrimType,
			uint32 InVSFormat,
			uint32 InVertexCount,
			E_INDEX_TYPE InIndexType,
			uint32 InIndexCount,
			const aabbox3df& InMeshBBox
		) = 0;
		virtual FInstanceBufferPtr CreateInstanceBuffer() = 0;
		virtual FInstanceBufferPtr CreateEmptyInstanceBuffer(uint32 InstanceCount, uint32 InstanceStride) = 0;
		virtual FPipelinePtr CreatePipeline(FShaderPtr InShader) = 0;
		virtual FRenderTargetPtr CreateRenderTarget(int32 W, int32 H) = 0;
		virtual FRenderResourceTablePtr CreateRenderResourceTable(uint32 InSize, E_RENDER_RESOURCE_HEAP_TYPE InHeap);
		virtual FShaderPtr CreateShader(const TShaderNames& InNames, E_SHADER_TYPE Type) = 0;
		virtual FShaderPtr CreateComputeShader(const TString& InComputeShaderName) = 0;
		virtual FArgumentBufferPtr CreateArgumentBuffer(int32 ReservedSlots) = 0;
		virtual FGPUCommandSignaturePtr CreateGPUCommandSignature(FPipelinePtr Pipeline, const TVector<E_GPU_COMMAND_TYPE>& CommandStructure) = 0;
		virtual FGPUCommandBufferPtr CreateGPUCommandBuffer(FGPUCommandSignaturePtr GPUCommandSignature, uint32 CommandsCount, uint32 Flag = 0) = 0;

		// RTX
		virtual bool UpdateHardwareResourceRtxPL(FRtxPipelinePtr Pipeline, TRtxPipelinePtr InPipelineDesc) = 0;
		virtual void SetRtxPipeline(FRtxPipelinePtr RtxPipeline) = 0;
		virtual void TraceRays(FRtxPipelinePtr RtxPipeline, const vector3di& Size) = 0;

		// Graphics and Compute
		virtual bool UpdateHardwareResourceMesh(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData) = 0;
		virtual bool UpdateHardwareResourceMesh(
			FMeshBufferPtr MeshBuffer, 
			uint32 VertexDataSize, 
			uint32 VertexDataStride, 
			uint32 IndexDataSize, 
			E_INDEX_TYPE IndexType,
			const TString& BufferName) = 0;
		virtual bool UpdateHardwareResourceIB(FInstanceBufferPtr InstanceBuffer, TInstanceBufferPtr InInstanceData) = 0;
		virtual bool UpdateHardwareResourceTexture(FTexturePtr Texture) = 0;
		virtual bool UpdateHardwareResourceTexture(FTexturePtr Texture, TTexturePtr InTexData) = 0;
		virtual bool UpdateHardwareResourceTexture(FTexturePtr Texture, TImagePtr InTexData) = 0;
		virtual bool UpdateHardwareResourcePL(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc) = 0;
        virtual bool UpdateHardwareResourceTilePL(FPipelinePtr Pipeline, TTilePipelinePtr InTilePipelineDesc) = 0;
		virtual bool UpdateHardwareResourceUB(FUniformBufferPtr UniformBuffer, const void* InData) = 0;
		virtual bool UpdateHardwareResourceRT(FRenderTargetPtr RenderTarget) = 0;
		virtual bool UpdateHardwareResourceShader(FShaderPtr ShaderResource, TShaderPtr InShaderSource) = 0;
		virtual bool UpdateHardwareResourceAB(FArgumentBufferPtr ArgumentBuffer, FShaderPtr InShader, int32 SpecifiedBindingIndex = -1) = 0;
		virtual bool UpdateHardwareResourceGPUCommandSig(FGPUCommandSignaturePtr GPUCommandSignature) = 0;
		virtual bool UpdateHardwareResourceGPUCommandBuffer(FGPUCommandBufferPtr GPUCommandBuffer) = 0;
		virtual void PrepareDataForCPU(FTexturePtr Texture) = 0;
		virtual void PrepareDataForCPU(FUniformBufferPtr UniformBuffer) = 0;
		virtual bool CopyTextureRegion(FTexturePtr DstTexture, const recti& InDstRegion, uint32 DstMipLevel, FTexturePtr SrcTexture, uint32 SrcMipLevel) = 0;
		virtual bool CopyBufferRegion(FUniformBufferPtr DstBuffer, uint32 DstOffset, FUniformBufferPtr SrcBuffer, uint32 Length) = 0;
		virtual bool CopyBufferRegion(
			FMeshBufferPtr DstBuffer, 
			uint32 DstVertexOffset, 
			uint32 DstIndexOffset,
			FMeshBufferPtr SrcBuffer, 
			uint32 SrcVertexOffset, 
			uint32 VertexLengthInBytes,
			uint32 SrcIndexOffset,
			uint32 IndexLengthInBytes) = 0;
		virtual bool CopyBufferRegion(
			FInstanceBufferPtr DstBuffer,
			uint32 DstInstanceOffset,
			FInstanceBufferPtr SrcBuffer,
			uint32 SrcInstanceOffset,
			uint32 InstanceCount) = 0;
		virtual bool CopyBufferRegion(
			FMeshBufferPtr DstBuffer,
			uint32 DstOffsetInBytes,
			FUniformBufferPtr SrcBuffer,
			uint32 SrcOffsetInBytes,
			uint32 Bytes) = 0;

		virtual void PutConstantBufferInHeap(FUniformBufferPtr InUniformBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) = 0;
		virtual void PutTextureInHeap(FTexturePtr InTexture, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) = 0;
		virtual void PutRWTextureInHeap(FTexturePtr InTexture, uint32 InMipLevel, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) = 0;
		virtual void PutUniformBufferInHeap(FUniformBufferPtr InBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) = 0;
		virtual void PutRWUniformBufferInHeap(FUniformBufferPtr InBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) = 0;
		virtual void PutMeshBufferInHeap(FMeshBufferPtr InBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, int32 InVBHeapSlot, int32 InIBHeapSlot) = 0;
		virtual void PutInstanceBufferInHeap(FInstanceBufferPtr InBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) = 0;
		virtual void PutRTColorInHeap(FTexturePtr InTexture, uint32 InHeapSlot) = 0;
		virtual void PutRTDepthInHeap(FTexturePtr InTexture, uint32 InHeapSlot) = 0;
		virtual void PutTopAccelerationStructureInHeap(FTopLevelAccelerationStructurePtr InTLAS, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) = 0;

		// Graphics
		virtual void SetGraphicsPipeline(FPipelinePtr InPipeline) = 0;
		virtual void SetMeshBuffer(FMeshBufferPtr InMeshBuffer, FInstanceBufferPtr InInstanceBuffer) = 0;
		virtual void SetMeshBufferAtSlot(uint32 StartSlot, FMeshBufferPtr InMeshBuffer) = 0;
		virtual void SetVertexBufferAtSlot(uint32 StartSlot, FMeshBufferPtr InMeshBuffer) = 0;
		virtual void SetIndexBufferFromMeshBuffer(FMeshBufferPtr InMeshBuffer) = 0;
		virtual void SetIndexBufferFromUniformBuffer(FUniformBufferPtr InIndexBuffer) = 0;
		virtual void SetInstanceBufferAtSlot(uint32 StartSlot, FInstanceBufferPtr InInstanceBuffer) = 0;
		virtual void SetUniformBuffer(E_SHADER_STAGE ShaderStage, int32 BindIndex, FUniformBufferPtr InUniformBuffer) = 0;
		virtual void SetRenderResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable) = 0;
		virtual void SetShaderTexture(int32 BindIndex, FTexturePtr InTexture) = 0;
		virtual void SetArgumentBuffer(int32 BindIndex, FArgumentBufferPtr InArgumentBuffer) = 0;

		virtual void SetResourceStateAS(FTopLevelAccelerationStructurePtr InAS, E_RESOURCE_STATE NewState, bool Immediate = true) = 0;
		virtual void SetResourceStateTexture(FTexturePtr InTexture, E_RESOURCE_STATE NewState, bool Immediate = true) = 0;
		virtual void SetResourceStateUB(FUniformBufferPtr InUniformBuffer, E_RESOURCE_STATE NewState, bool Immediate = true) = 0;
		virtual void SetResourceStateCB(FGPUCommandBufferPtr InCommandBuffer, E_RESOURCE_STATE NewState, bool Immediate = true) = 0;
		virtual void SetResourceStateInsB(FInstanceBufferPtr InInstanceBuffer, E_RESOURCE_STATE NewState, bool Immediate = true) = 0;
		virtual void SetResourceStateMB(FMeshBufferPtr InMeshBuffer, E_RESOURCE_STATE NewState, bool Immediate = true) = 0;
		virtual void FlushResourceStateChange() = 0;

		virtual void SetStencilRef(uint32 InRefValue) = 0;
		virtual void DrawPrimitiveInstanced(FMeshBufferPtr MeshBuffer, uint32 InstanceCount, uint32 InstanceOffset) = 0;
		virtual void DrawPrimitiveIndexedInstanced(FMeshBufferPtr MeshBuffer, uint32 InstanceCount, uint32 InstanceOffset) = 0;
		virtual void DrawPrimitiveIndexedInstanced(FMeshBufferPtr MeshBuffer, uint32 IndicesCount, uint32 InstanceCount, uint32 IndexOffset, uint32 InstanceOffset) = 0;
		virtual void GraphicsCopyBuffer(FUniformBufferPtr Dest, uint32 DestOffset, FUniformBufferPtr Src, uint32 SrcOffset, uint32 CopySize) = 0;
        
        // Tile
        virtual void SetTilePipeline(FPipelinePtr InPipeline) = 0;
        virtual void SetTileBuffer(int32 BindIndex, FUniformBufferPtr InUniformBuffer) = 0;
        virtual void DispatchTile(const vector3di& GroupSize) = 0;

		// Compute
		virtual void SetComputePipeline(FPipelinePtr InPipeline) = 0;
		virtual void SetComputeConstant(int32 BindIndex, const FUInt4& InValue) = 0;
		virtual void SetComputeConstant(int32 BindIndex, const FFloat4& InValue) = 0;
		virtual void SetComputeConstantBuffer(int32 BindIndex, FUniformBufferPtr InUniformBuffer, uint32 BufferOffset = 0) = 0;
		virtual void SetComputeShaderResource(int32 BindIndex, FUniformBufferPtr InUniformBuffer, uint32 BufferOffset = 0) = 0;
		virtual void SetComputeShaderResource(int32 VertexBindIndex, int32 IndexBindIndex, FMeshBufferPtr InMeshBuffer, uint32 VertexBufferOffset = 0, uint32 IndexBufferOffset = 0) = 0;
		virtual void SetComputeResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable) = 0;
        virtual void SetComputeArgumentBuffer(int32 BindIndex, FArgumentBufferPtr InArgumentBuffer) = 0;
        virtual void SetComputeTexture(int32 BindIndex, FTexturePtr InTexture) = 0;
        
		virtual void DispatchCompute(const vector3di& GroupSize, const vector3di& GroupCount) = 0;
		virtual void ComputeCopyBuffer(FUniformBufferPtr Dest, uint32 DestOffset, FUniformBufferPtr Src, uint32 SrcOffset, uint32 CopySize) = 0;

		// GPU Command buffer
		virtual void ExecuteGPUDrawCommands(FGPUCommandBufferPtr GPUCommandBuffer) = 0;
		virtual void ExecuteGPUComputeCommands(FGPUCommandBufferPtr GPUCommandBuffer) = 0;

		virtual void SetViewport(const FViewport& InViewport);
		virtual void BeginRenderToRenderTarget(FRenderTargetPtr RT, const int8* PassName = "UnnamedPass", uint32 MipLevel = 0, const SColor& ClearColor = SColor(0, 0, 0, 0));

		E_RHI_TYPE GetRHIType() const
		{
			return RHIType;
		}

		const FViewport& GetViewport() const
		{
			return Viewport;
		}

		FRenderResourceHeap& GetRenderResourceHeap(int32 Index)
		{
			TI_ASSERT(Index >= 0 && Index < EHT_COUNT);
			return RenderResourceHeap[Index];
		}

	protected:
		static FRHI* RHI;
		FRHI(E_RHI_TYPE InRHIType);
		virtual ~FRHI();

		virtual void FeatureCheck() = 0;
		virtual void SupportFeature(E_RHI_FEATURE InFeature);
		
		static void GPUFrameDone()
		{
			++NumGPUFrames;
		}
		// Frames count that GPU done
		static TI_API uint32 NumGPUFrames;
	protected:
		E_RHI_TYPE RHIType;
		FViewport Viewport;
		FFrameResources * FrameResources[FRHIConfig::FrameBufferNum];

		FRenderTargetPtr CurrentRenderTarget;
		FViewport RtViewport;

		FRenderResourceHeap RenderResourceHeap[EHT_COUNT];
		FBoundResource CurrentBoundResource;
        
        struct FCommandListState
        {
            E_PIPELINE_TYPE ListType;
            int32 ListIndex;
            
            FCommandListState()
            : ListType(EPL_INVALID)
            , ListIndex(-1)
            {}
            
            void Reset()
            {
                ListType = EPL_INVALID;
                ListIndex = -1;
            }
        };
        FCommandListState CurrentCommandListState;
        TVector<FCommandListState> ListExecuteOrder;
        int32 CurrentCommandListCounter[EPL_NUM];
	};
}
