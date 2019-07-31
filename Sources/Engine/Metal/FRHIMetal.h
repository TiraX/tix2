/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_METAL
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>

namespace tix
{
	// Render hardware interface use Metal
	class FRHIMetal : public FRHI
	{
	public:
		virtual ~FRHIMetal();
        
		// RHI common methods
		virtual void InitRHI() override;
		virtual void BeginFrame() override;
		virtual void EndFrame() override;
        virtual void BeginRenderToFrameBuffer() override;

        virtual void InitCommandLists(uint32 NumGraphicsList, uint32 NumComputeList) override;
        virtual void BeginPopulateCommandList(E_PIPELINE_TYPE PipelineType) override;
        virtual void EndPopulateCommandList() override;
        
        virtual int32 GetCurrentEncodingFrameIndex() override;
		virtual void WaitingForGpu() override;

		virtual FTexturePtr CreateTexture() override;
		virtual FTexturePtr CreateTexture(const TTextureDesc& Desc) override;
		virtual FUniformBufferPtr CreateUniformBuffer(uint32 InStructSizeInBytes, uint32 Elements, uint32 Flag = 0) override;
		virtual FMeshBufferPtr CreateMeshBuffer() override;
        virtual FInstanceBufferPtr CreateInstanceBuffer() override;
		virtual FPipelinePtr CreatePipeline(FShaderPtr InShader) override;
		virtual FRenderTargetPtr CreateRenderTarget(int32 W, int32 H) override;
        virtual FShaderPtr CreateShader(const TShaderNames& InNames) override;
        virtual FShaderPtr CreateComputeShader(const TString& InComputeShaderName) override;
        virtual FArgumentBufferPtr CreateArgumentBuffer(FShaderPtr InShader) override;

		virtual bool UpdateHardwareResourceMesh(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData) override;
        virtual bool UpdateHardwareResourceIB(FInstanceBufferPtr InstanceBuffer, TInstanceBufferPtr InInstanceData) override;
		virtual bool UpdateHardwareResourceTexture(FTexturePtr Texture) override;
		virtual bool UpdateHardwareResourceTexture(FTexturePtr Texture, TTexturePtr InTexData) override;
        virtual bool UpdateHardwareResourceTexture(FTexturePtr Texture, TImagePtr InTexData) override;
        virtual bool UpdateHardwareResourceTextureRegion(FTexturePtr DestTexture, FTexturePtr SrcTexture, const recti& InRegion) override;
		virtual bool UpdateHardwareResourcePL(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc) override;
		virtual bool UpdateHardwareResourceUB(FUniformBufferPtr UniformBuffer, void* InData) override;
        virtual bool UpdateHardwareResourceRT(FRenderTargetPtr RenderTarget) override;
        virtual bool UpdateHardwareResourceShader(FShaderPtr ShaderResource, TShaderPtr InShaderSource) override;
        virtual bool UpdateHardwareResourceAB(FArgumentBufferPtr ArgumentBuffer, TStreamPtr ArgumentData, const TVector<FTexturePtr>& ArgumentTextures) override;
        virtual void PrepareDataForCPU(FUniformBufferPtr UniformBuffer) override;
        
		virtual void PutConstantBufferInHeap(FUniformBufferPtr InUniformBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) override;
		virtual void PutTextureInHeap(FTexturePtr InTexture, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) override;
        virtual void PutBufferInHeap(FUniformBufferPtr InBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) override;
		virtual void PutRTColorInHeap(FTexturePtr InTexture, uint32 InHeapSlot) override;
		virtual void PutRTDepthInHeap(FTexturePtr InTexture, uint32 InHeapSlot) override;

        // Graphics
        virtual void SetGraphicsPipeline(FPipelinePtr InPipeline) override;
		virtual void SetMeshBuffer(FMeshBufferPtr InMeshBuffer, FInstanceBufferPtr InInstanceBuffer) override;
		virtual void SetUniformBuffer(E_SHADER_STAGE ShaderStage, int32 BindIndex, FUniformBufferPtr InUniformBuffer) override;
		virtual void SetRenderResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable) override;
		virtual void SetShaderTexture(int32 BindIndex, FTexturePtr InTexture) override;
        virtual void SetArgumentBuffer(FArgumentBufferPtr InArgumentBuffer) override;
        virtual void SetResourceStateUB(FUniformBufferPtr InUniformBuffer, E_RESOURCE_STATE NewState) override;

		virtual void SetStencilRef(uint32 InRefValue) override;
		virtual void DrawPrimitiveIndexedInstanced(FMeshBufferPtr MeshBuffer, uint32 InstanceCount) override;
        virtual void GraphicsCopyBuffer(FUniformBufferPtr Dest, uint32 DestOffset, FUniformBufferPtr Src, uint32 SrcOffset, uint32 CopySize) override;

        // Compute
        virtual void SetComputePipeline(FPipelinePtr InPipeline) override;
        virtual void SetComputeConstantBuffer(int32 BindIndex, FUniformBufferPtr InUniformBuffer) override;
        virtual void SetComputeResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable) override;
        
        virtual void DispatchCompute(uint32 GroupCountX, uint32 GroupCountY, uint32 GroupCountZ) override;
        virtual void ComputeCopyBuffer(FUniformBufferPtr Dest, uint32 DestOffset, FUniformBufferPtr Src, uint32 SrcOffset, uint32 CopySize) override;
        
		virtual void SetViewport(const FViewport& InViewport) override;
		virtual void PushRenderTarget(FRenderTargetPtr RT, const int8* PassName = "UnnamedPass") override;
		virtual FRenderTargetPtr PopRenderTarget() override;

	protected: 
		FRHIMetal();
        
        void HoldResourceReference(FRenderResourcePtr InResource);

	private:
        CAMetalLayer * MtlLayer;
        id <MTLDevice> MtlDevice;
        id <CAMetalDrawable> CurrentDrawable;
        
        id <MTLCommandQueue> CommandQueue;
        id <MTLLibrary> DefaultLibrary;
        
        id <MTLCommandBuffer> CommandBuffer;
        id <MTLRenderCommandEncoder> RenderEncoder;
        
        MTLRenderPassDescriptor* FrameBufferPassDesc;
        
        int32 LastFrameMark;
        int32 CurrentFrame;
        dispatch_semaphore_t InflightSemaphore;
        
		friend class FRHI;
	};
}
#endif	// COMPILE_WITH_RHI_METAL
