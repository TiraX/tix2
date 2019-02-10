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

		virtual void WaitingForGpu() override;

		virtual FTexturePtr CreateTexture() override;
		virtual FTexturePtr CreateTexture(const TTextureDesc& Desc) override;
		virtual FUniformBufferPtr CreateUniformBuffer(uint32 InStructSize) override;
		virtual FMeshBufferPtr CreateMeshBuffer() override;
		virtual FPipelinePtr CreatePipeline(FShaderPtr InShader) override;
		virtual FRenderTargetPtr CreateRenderTarget(int32 W, int32 H) override;
        virtual FShaderPtr CreateShader(const TShaderNames& ShaderNames) override;
        virtual FArgumentBufferPtr CreateArgumentBuffer(FShaderPtr InShader) override;

		virtual bool UpdateHardwareResource(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData) override;
		virtual bool UpdateHardwareResource(FTexturePtr Texture) override;
		virtual bool UpdateHardwareResource(FTexturePtr Texture, TTexturePtr InTexData) override;
		virtual bool UpdateHardwareResource(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc) override;
		virtual bool UpdateHardwareResource(FUniformBufferPtr UniformBuffer, void* InData) override;
        virtual bool UpdateHardwareResource(FRenderTargetPtr RenderTarget) override;
        virtual bool UpdateHardwareResource(FShaderPtr ShaderResource) override;
        virtual bool UpdateHardwareResource(FArgumentBufferPtr ArgumentBuffer, TStreamPtr ArgumentData, const TVector<FTexturePtr>& ArgumentTextures) override;

		virtual void PutUniformBufferInHeap(FUniformBufferPtr InUniformBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) override;
		virtual void PutTextureInHeap(FTexturePtr InTexture, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) override;
		virtual void PutRTColorInHeap(FTexturePtr InTexture, uint32 InHeapSlot) override;
		virtual void PutRTDepthInHeap(FTexturePtr InTexture, uint32 InHeapSlot) override;

		virtual void SetPipeline(FPipelinePtr InPipeline) override;
		virtual void SetMeshBuffer(FMeshBufferPtr InMeshBuffer) override;
		virtual void SetUniformBuffer(E_SHADER_STAGE ShaderStage, int32 BindIndex, FUniformBufferPtr InUniformBuffer) override;
		virtual void SetRenderResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable) override;
		virtual void SetShaderTexture(int32 BindIndex, FTexturePtr InTexture) override;
        virtual void SetArgumentBuffer(FArgumentBufferPtr InArgumentBuffer) override;

		virtual void SetStencilRef(uint32 InRefValue) override;
		virtual void DrawPrimitiveIndexedInstanced(FMeshBufferPtr MeshBuffer, uint32 InstanceCount) override;

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
