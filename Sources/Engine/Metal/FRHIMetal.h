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
    class FFrameResourcesMetal;
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

        virtual void BeginComputeTask() override;
        virtual void EndComputeTask() override;
        
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
        virtual FArgumentBufferPtr CreateArgumentBuffer(int32 ReservedSlots) override;

		virtual bool UpdateHardwareResourceMesh(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData) override;
        virtual bool UpdateHardwareResourceIB(FInstanceBufferPtr InstanceBuffer, TInstanceBufferPtr InInstanceData) override;
		virtual bool UpdateHardwareResourceTexture(FTexturePtr Texture) override;
		virtual bool UpdateHardwareResourceTexture(FTexturePtr Texture, TTexturePtr InTexData) override;
        virtual bool UpdateHardwareResourceTexture(FTexturePtr Texture, TImagePtr InTexData) override;
		virtual bool UpdateHardwareResourcePL(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc) override;
		virtual bool UpdateHardwareResourceUB(FUniformBufferPtr UniformBuffer, const void* InData) override;
        virtual bool UpdateHardwareResourceRT(FRenderTargetPtr RenderTarget) override;
        virtual bool UpdateHardwareResourceShader(FShaderPtr ShaderResource, TShaderPtr InShaderSource) override;
        virtual bool UpdateHardwareResourceAB(FArgumentBufferPtr ArgumentBuffer, FShaderPtr InShader, int32 SpecifiedBindingIndex) override;
        virtual void PrepareDataForCPU(FUniformBufferPtr UniformBuffer) override;
        virtual bool CopyTextureRegion(FTexturePtr DstTexture, const recti& InDstRegion, FTexturePtr SrcTexture) override;
        virtual bool CopyBufferRegion(FUniformBufferPtr DstBuffer, uint32 DstOffset, FUniformBufferPtr SrcBuffer, uint32 Length) override;
        
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
        virtual void SetArgumentBuffer(int32 InBindIndex, FArgumentBufferPtr InArgumentBuffer) override;
        virtual void SetResourceStateUB(FUniformBufferPtr InUniformBuffer, E_RESOURCE_STATE NewState) override;

		virtual void SetStencilRef(uint32 InRefValue) override;
		virtual void DrawPrimitiveIndexedInstanced(FMeshBufferPtr MeshBuffer, uint32 InstanceCount) override;
        virtual void GraphicsCopyBuffer(FUniformBufferPtr Dest, uint32 DestOffset, FUniformBufferPtr Src, uint32 SrcOffset, uint32 CopySize) override;

        // Compute
        virtual void SetComputePipeline(FPipelinePtr InPipeline) override;
        virtual void SetComputeConstantBuffer(int32 BindIndex, FUniformBufferPtr InUniformBuffer) override;
        virtual void SetComputeResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable) override;
        virtual void SetComputeArgumentBuffer(int32 BindIndex, FArgumentBufferPtr InArgumentBuffer) override;
        
        virtual void DispatchCompute(const vector3di& GroupSize, const vector3di& GroupCount) override;
        virtual void ComputeCopyBuffer(FUniformBufferPtr Dest, uint32 DestOffset, FUniformBufferPtr Src, uint32 SrcOffset, uint32 CopySize) override;
        
		virtual void SetViewport(const FViewport& InViewport) override;
		virtual void BeginRenderToRenderTarget(FRenderTargetPtr RT, const int8* PassName = "UnnamedPass") override;

	protected: 
		FRHIMetal();
        
        MTLTextureDescriptor* CreateDescriptorFromTextureDesc(const TTextureDesc& InDesc);
        MTLTextureDescriptor* CreateDescriptorFromTexture(id<MTLTexture> InTexture);
        
        void CopyTextureData(id<MTLTexture> DstTexture, id<MTLTexture> SrcTexture);
        
        id<MTLTexture> CloneTextureInHeap(id<MTLTexture> InTexture);
        id<MTLBuffer> CloneBufferToHeap(id<MTLBuffer> InBuffer);
        
        void HoldResourceReference(FRenderResourcePtr InResource);
        void HoldResourceReference(id<MTLBuffer> InBuffer);
        void HoldResourceReference(id<MTLTexture> InTexture);
        // Close current encoder, if next encoder do not match current one.
        // Return true if current encoder closed
        // Return false if none encoder closed
        bool CloseCurrentEncoderIfNotMatch(E_PIPELINE_TYPE NextPipelineType);
        id <MTLBlitCommandEncoder> RequestBlitEncoder();
        
        void ClearBindedBuffers();

	private:
        CAMetalLayer * MtlLayer;
        id <MTLDevice> MtlDevice;
        id <CAMetalDrawable> CurrentDrawable;
        
        id <MTLCommandQueue> CommandQueue;
        id <MTLLibrary> DefaultLibrary;
        
        id <MTLCommandBuffer> CommandBuffer;
        id <MTLHeap> ResourceHeap;
        
        id <MTLRenderCommandEncoder> RenderEncoder;
        id <MTLComputeCommandEncoder> ComputeEncoder;
        id <MTLBlitCommandEncoder> BlitEncoder;
        
        MTLRenderPassDescriptor* FrameBufferPassDesc;
        
        int32 LastFrameMark;
        int32 CurrentFrame;
        dispatch_semaphore_t InflightSemaphore;
        
        // Frame on the fly resource holders
        FFrameResourcesMetal * ResHolders[FRHIConfig::FrameBufferNum];
        
        // Binded buffer in a frame
        static const int32 MaxBindingBuffers = 32;
        id <MTLBuffer> VSBindedBuffers[MaxBindingBuffers];
        id <MTLBuffer> PSBindedBuffers[MaxBindingBuffers];
        
		friend class FRHI;
	};
}
#endif	// COMPILE_WITH_RHI_METAL
