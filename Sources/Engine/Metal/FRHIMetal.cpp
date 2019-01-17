/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIMetal.h"

#if COMPILE_WITH_RHI_METAL
#include "TDeviceIOS.h"
#import "FMetalView.h"
#include "FRHIMetalConversion.h"
#include "FTextureMetal.h"
#include "FShaderBindingMetal.h"
#include "FPipelineMetal.h"
#include "FMeshBufferMetal.h"
#include "FShaderMetal.h"

namespace tix
{
	FRHIMetal::FRHIMetal()
		: FRHI(ERHI_METAL)
	{
	}

	FRHIMetal::~FRHIMetal()
	{
        _LOG(Log, "  RHI Metal destroy.\n");
	}

	void FRHIMetal::InitRHI()
	{
        FMetalView * MetalView = [FMetalView sharedMetalView];
        TI_ASSERT(MetalView != nil);
        
        //int32 w = (int32)(MetalLayer.bounds.size.width);
        //int32 h = (int32)(MetalLayer.bounds.size.height);
        
        // Grab a metal device
        MtlDevice = MetalView.MtlDevice;
        
        // Create Command Queue
        CommandQueue = [MtlDevice newCommandQueue];
        
        // Create Metal Default Library
        NSFileManager* fm = [NSFileManager defaultManager];
        NSString* curr_path = [NSString stringWithString: fm.currentDirectoryPath];
        [fm changeCurrentDirectoryPath:@"/"];
        DefaultLibrary = [MtlDevice newDefaultLibrary];
        [fm changeCurrentDirectoryPath:curr_path];
        
        InflightSemaphore = dispatch_semaphore_create(FRHIConfig::FrameBufferNum);
        
		_LOG(Log, "  RHI Metal inited.\n");
	}
    
	void FRHIMetal::BeginFrame()
    {
        TI_ASSERT(CommandBuffer == nil);
        TI_ASSERT(RenderEncoder == nil);
        
        dispatch_semaphore_wait(InflightSemaphore, DISPATCH_TIME_FOREVER);
        
        CommandBuffer = [CommandQueue commandBuffer];
	}

	void FRHIMetal::EndFrame()
    {
        TI_ASSERT(CommandBuffer != nil);
        TI_ASSERT(RenderEncoder != nil);
        TI_ASSERT(CurrentDrawable != nil);
        
        [RenderEncoder endEncoding];
        
        // call the view's completion handler which is required by the view since it will signal its semaphore and set up the next buffer
        __block dispatch_semaphore_t block_sema = InflightSemaphore;
        [CommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
            
            // GPU has completed rendering the frame and is done using the contents of any buffers previously encoded on the CPU for that frame.
            // Signal the semaphore and allow the CPU to proceed and construct the next frame.
            dispatch_semaphore_signal(block_sema);
        }];
        
        [CommandBuffer presentDrawable:CurrentDrawable];
        [CommandBuffer commit];
        
        RenderEncoder = nil;
        CommandBuffer = nil;
        CurrentDrawable = nil;
	}

	FTexturePtr FRHIMetal::CreateTexture()
	{
        return ti_new FTextureMetal;
	}

	FTexturePtr FRHIMetal::CreateTexture(const TTextureDesc& Desc)
	{
        TI_ASSERT(0);
        return nullptr;
	}

	FUniformBufferPtr FRHIMetal::CreateUniformBuffer(uint32 InStructSize)
	{
        TI_ASSERT(0);
        return nullptr;
	}

	FMeshBufferPtr FRHIMetal::CreateMeshBuffer()
	{
        return ti_new FMeshBufferMetal;
	}

	FPipelinePtr FRHIMetal::CreatePipeline()
	{
        return ti_new FPipelineMetal;
	}

	//FRenderTargetPtr FRHIMetal::CreateRenderTarget(int32 W, int32 H)
	//{
	//	return ti_new FRenderTargetDx12(W, H);
	//}

	FShaderBindingPtr FRHIMetal::CreateShaderBinding(uint32 NumBindings)
	{
        return ti_new FShaderBindingMetal(NumBindings);
	}
    
    FShaderPtr FRHIMetal::CreateShader(const TShaderNames& ShaderNames)
    {
        return ti_new FShaderMetal(ShaderNames);
    }

	// Wait for pending GPU work to complete.
	void FRHIMetal::WaitingForGpu()
	{
        TI_ASSERT(0);
	}

	bool FRHIMetal::UpdateHardwareResource(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData)
	{
        TI_ASSERT(0);
		return true;
	}

    
	bool FRHIMetal::UpdateHardwareResource(FTexturePtr Texture)
	{
        TI_ASSERT(0);

		return true;
	}

	bool FRHIMetal::UpdateHardwareResource(FTexturePtr Texture, TTexturePtr InTexData)
	{
        TI_ASSERT(0);
		return true;
	}

	bool FRHIMetal::UpdateHardwareResource(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc)
    {
        FPipelineMetal * PipelineMetal = static_cast<FPipelineMetal*>(Pipeline.get());
        const TPipelineDesc& Desc = InPipelineDesc->GetDesc();
        
        MTLRenderPipelineDescriptor * PipelineStateDesc = [[MTLRenderPipelineDescriptor alloc] init];
#if defined (TIX_DEBUG)
        Pipeline->SetResourceName(InPipelineDesc->GetResourceName());
        NSString * PipelineName = [NSString stringWithUTF8String:InPipelineDesc->GetResourceName().c_str()];
        PipelineStateDesc.label = PipelineName;
#endif
        PipelineStateDesc.sampleCount = 1;
        
        // Load vertex function and pixel function
        id <MTLFunction> VertexProgram = nil, FragmentProgram = nil;
        FShaderPtr Shader = InPipelineDesc->GetDesc().Shader->ShaderResource;
        NSString * VertexShader = nil;
        TI_ASSERT(!Shader->GetShaderName(ESS_VERTEX_SHADER).empty());
        VertexShader = [NSString stringWithUTF8String:Shader->GetShaderName(ESS_VERTEX_SHADER).c_str()];
        NSString * FragmentShader = nil;
        if (!Shader->GetShaderName(ESS_PIXEL_SHADER).empty())
        {
            FragmentShader = [NSString stringWithUTF8String:Shader->GetShaderName(ESS_PIXEL_SHADER).c_str()];
        }
        
        VertexProgram = [DefaultLibrary newFunctionWithName:VertexShader];
        if(VertexProgram == nil)
        {
            _LOG(Fatal, "Can not load vertex function %s.\n", Shader->GetShaderName(ESS_VERTEX_SHADER).c_str());
            TI_ASSERT(0);
        }
        
        if (FragmentShader != nil)
        {
            FragmentProgram = [DefaultLibrary newFunctionWithName:FragmentShader];
        }
        
        PipelineStateDesc.vertexFunction = VertexProgram;
        PipelineStateDesc.fragmentFunction = FragmentProgram;
        
        // Set vertex layout
        TVector<E_MESH_STREAM_INDEX> Streams = TMeshBuffer::GetSteamsFromFormat(Desc.VsFormat);
        MTLVertexDescriptor * VertexDesc = [[MTLVertexDescriptor alloc] init];
        
        uint32 VertexDataOffset = 0;
        for (uint32 i = 0; i < (uint32)Streams.size(); ++i)
        {
            E_MESH_STREAM_INDEX Stream = Streams[i];
            VertexDesc.attributes[i].format = k_MESHBUFFER_STREAM_FORMAT_MAP[Stream];
            VertexDesc.attributes[i].bufferIndex = 0;
            VertexDesc.attributes[i].offset = VertexDataOffset;
            VertexDataOffset += TMeshBuffer::SemanticSize[Stream];
        }
        VertexDesc.layouts[0].stride = TMeshBuffer::GetStrideFromFormat(Desc.VsFormat);
        VertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        PipelineStateDesc.vertexDescriptor = VertexDesc;
        
        // Set color, depth, stencil attachments format
        TI_ASSERT(Desc.RTCount > 0);
        for (int32 r = 0 ; r < Desc.RTCount; ++r)
        {
            PipelineStateDesc.colorAttachments[0].pixelFormat = GetMetalPixelFormat(Desc.RTFormats[r]);
        }
        if (Desc.DepthFormat != EPF_UNKNOWN)
        {
            PipelineStateDesc.depthAttachmentPixelFormat = GetMetalPixelFormat(Desc.DepthFormat);
        }
        else
        {
            PipelineStateDesc.depthAttachmentPixelFormat = MTLPixelFormatInvalid;
        }
        if (Desc.StencilFormat != EPF_UNKNOWN)
        {
            PipelineStateDesc.stencilAttachmentPixelFormat = GetMetalPixelFormat(Desc.StencilFormat);
        }
        else
        {
            PipelineStateDesc.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
        }
        
        for (int32 r = 0 ; r < Desc.RTCount; ++r)
        {
            if (Desc.IsEnabled(EPSO_BLEND))
            {
                PipelineStateDesc.colorAttachments[r].blendingEnabled = YES;
                PipelineStateDesc.colorAttachments[r].rgbBlendOperation = k_BLEND_OPERATION_MAP[Desc.BlendState.BlendOp];
                PipelineStateDesc.colorAttachments[r].alphaBlendOperation = k_BLEND_OPERATION_MAP[Desc.BlendState.BlendOpAlpha];
                PipelineStateDesc.colorAttachments[r].sourceRGBBlendFactor = k_BLEND_FACTOR_MAP[Desc.BlendState.SrcBlend];
                PipelineStateDesc.colorAttachments[r].sourceAlphaBlendFactor = k_BLEND_FACTOR_MAP[Desc.BlendState.SrcBlendAlpha];
                PipelineStateDesc.colorAttachments[r].destinationRGBBlendFactor = k_BLEND_FACTOR_MAP[Desc.BlendState.DestBlend];
                PipelineStateDesc.colorAttachments[r].destinationAlphaBlendFactor = k_BLEND_FACTOR_MAP[Desc.BlendState.DestBlendAlpha];
            }
            else
            {
                PipelineStateDesc.colorAttachments[r].blendingEnabled = NO;
            }
        }
        
        NSError* Err  = nil;
        //MTLRenderPipelineReflection * ReflectionObj = nil;
        //PipelineMetal->PipelineState = [MtlDevice newRenderPipelineStateWithDescriptor : PipelineStateDescriptor options:MTLPipelineOptionArgumentInfo reflection:&ReflectionObj error:&Err];
        PipelineMetal->PipelineState = [MtlDevice newRenderPipelineStateWithDescriptor : PipelineStateDesc options:MTLPipelineOptionNone reflection:nil error:&Err];
        
        MTLDepthStencilDescriptor * DepthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
        DepthStateDesc.depthCompareFunction = k_COMPARE_FUNC_MAP[Desc.DepthStencilDesc.DepthFunc];
        DepthStateDesc.depthWriteEnabled = Desc.IsEnabled(EPSO_DEPTH);
        
        if (Desc.IsEnabled(EPSO_STENCIL))
        {
            DepthStateDesc.frontFaceStencil.stencilCompareFunction = k_COMPARE_FUNC_MAP[Desc.DepthStencilDesc.FrontFace.StencilFunc];
            DepthStateDesc.frontFaceStencil.stencilFailureOperation = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.FrontFace.StencilFailOp];
            DepthStateDesc.frontFaceStencil.depthFailureOperation = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.FrontFace.StencilDepthFailOp];
            DepthStateDesc.frontFaceStencil.depthStencilPassOperation = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.FrontFace.StencilPassOp];
            DepthStateDesc.frontFaceStencil.readMask = Desc.DepthStencilDesc.StencilReadMask;
            DepthStateDesc.frontFaceStencil.writeMask = Desc.DepthStencilDesc.StencilWriteMask;
            
            DepthStateDesc.backFaceStencil.stencilCompareFunction = k_COMPARE_FUNC_MAP[Desc.DepthStencilDesc.BackFace.StencilFunc];
            DepthStateDesc.backFaceStencil.stencilFailureOperation = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.BackFace.StencilFailOp];
            DepthStateDesc.backFaceStencil.depthFailureOperation = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.BackFace.StencilDepthFailOp];
            DepthStateDesc.backFaceStencil.depthStencilPassOperation = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.BackFace.StencilPassOp];
            DepthStateDesc.backFaceStencil.readMask = Desc.DepthStencilDesc.StencilReadMask;
            DepthStateDesc.backFaceStencil.writeMask = Desc.DepthStencilDesc.StencilWriteMask;
        }
        
        PipelineMetal->DepthState = [MtlDevice newDepthStencilStateWithDescriptor:DepthStateDesc];
        
		return true;
	}

	//static const int32 UniformBufferAlignSize = 256;
	bool FRHIMetal::UpdateHardwareResource(FUniformBufferPtr UniformBuffer, void* InData)
	{
        TI_ASSERT(0);

		return true;
	}

	bool FRHIMetal::UpdateHardwareResource(FShaderBindingPtr ShaderBindingResource, const TVector<TBindingParamInfo>& BindingInfos)
	{
        //TI_ASSERT(0);
		return true;
	}
    
    bool FRHIMetal::UpdateHardwareResource(FShaderPtr ShaderResource)
    {
        // Metal shader only need names. Do not need hard ware resource.
        return true;
    }
    
	void FRHIMetal::PutUniformBufferInHeap(FUniformBufferPtr InUniformBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot)
	{
        TI_ASSERT(0);
	}

	void FRHIMetal::PutTextureInHeap(FTexturePtr InTexture, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot)
	{
        TI_ASSERT(0);
	}

	void FRHIMetal::PutRTColorInHeap(FTexturePtr InTexture, uint32 InHeapSlot)
	{
        TI_ASSERT(0);
	}

	void FRHIMetal::PutRTDepthInHeap(FTexturePtr InTexture, uint32 InHeapSlot)
	{
        TI_ASSERT(0);
	}

	void FRHIMetal::SetPipeline(FPipelinePtr InPipeline)
	{
        TI_ASSERT(0);
	}

	void FRHIMetal::SetMeshBuffer(FMeshBufferPtr InMeshBuffer)
	{
        TI_ASSERT(0);
	}

	void FRHIMetal::SetUniformBuffer(int32 BindIndex, FUniformBufferPtr InUniformBuffer)
	{
        TI_ASSERT(0);
	}

	void FRHIMetal::SetRenderResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable)
	{
        TI_ASSERT(0);
	}

	void FRHIMetal::SetShaderTexture(int32 BindIndex, FTexturePtr InTexture)
	{
        TI_ASSERT(0);
	}

	void FRHIMetal::SetStencilRef(uint32 InRefValue)
	{
        TI_ASSERT(0);
	}

	void FRHIMetal::DrawPrimitiveIndexedInstanced(
		uint32 IndexCountPerInstance,
		uint32 InstanceCount,
		uint32 StartIndexLocation,
		int32 BaseVertexLocation,
		uint32 StartInstanceLocation)
	{
        TI_ASSERT(0);
	}

	void FRHIMetal::SetViewport(const FViewport& VP)
	{
		FRHI::SetViewport(VP);
        TI_ASSERT(0);
	}

	void FRHIMetal::PushRenderTarget(FRenderTargetPtr RT)
	{
		FRHI::PushRenderTarget(RT);

        TI_ASSERT(0);
	}

	FRenderTargetPtr FRHIMetal::PopRenderTarget()
	{
        TI_ASSERT(0);
        return nullptr;
	}
}
#endif	// COMPILE_WITH_RHI_METAL
