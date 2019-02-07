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
#include "FArgumentBufferMetal.h"
#include "FRenderTargetMetal.h"

namespace tix
{
	FRHIMetal::FRHIMetal()
		: FRHI(ERHI_METAL)
        , CurrentFrame(0)
	{
        // Create frame resource holders
        for (int32 i = 0 ; i < FRHIConfig::FrameBufferNum; ++ i)
        {
            FrameResources[i] = ti_new FFrameResources;
        }
	}

	FRHIMetal::~FRHIMetal()
	{
        _LOG(Log, "  RHI Metal destroy.\n");
        // Delete frame resource holders
        for (int32 i = 0; i < FRHIConfig::FrameBufferNum; ++i)
        {
            ti_delete FrameResources[i];
            FrameResources[i] = nullptr;
        }
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
            FrameResources[CurrentFrame]->RemoveAllReferences();
            CurrentFrame = (CurrentFrame + 1) % FRHIConfig::FrameBufferNum;
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
        return ti_new FTextureMetal(Desc);
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

	FPipelinePtr FRHIMetal::CreatePipeline(FShaderPtr InShader)
	{
        return ti_new FPipelineMetal(InShader);
	}

	FRenderTargetPtr FRHIMetal::CreateRenderTarget(int32 W, int32 H)
	{
		return ti_new FRenderTargetMetal(W, H);
	}
    
    FShaderPtr FRHIMetal::CreateShader(const TShaderNames& ShaderNames)
    {
        return ti_new FShaderMetal(ShaderNames);
    }
    
    FArgumentBufferPtr FRHIMetal::CreateArgumentBuffer(FShaderPtr InShader)
    {
        return ti_new FArgumentBufferMetal(InShader);
    }

	// Wait for pending GPU work to complete.
	void FRHIMetal::WaitingForGpu()
	{
        TI_ASSERT(0);
	}

	bool FRHIMetal::UpdateHardwareResource(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData)
	{
#if defined (TIX_DEBUG)
        MeshBuffer->SetResourceName(InMeshData->GetResourceName());
#endif
        FMeshBufferMetal * MBMetal = static_cast<FMeshBufferMetal*>(MeshBuffer.get());
        
        // Create Vertex Buffer
        const int32 BufferSize = InMeshData->GetVerticesCount() * InMeshData->GetStride();
        MBMetal->VertexBuffer = [MtlDevice newBufferWithBytes:InMeshData->GetVSData() length:BufferSize options:MTLResourceCPUCacheModeDefaultCache];
        
        const uint32 IndexBufferSize = (InMeshData->GetIndicesCount() * (InMeshData->GetIndexType() == EIT_16BIT ? sizeof(uint16) : sizeof(uint32)));
        MBMetal->IndexBuffer = [MtlDevice newBufferWithBytes:InMeshData->GetPSData() length:IndexBufferSize options:MTLResourceCPUCacheModeDefaultCache];
        
        // Hold resources used here
        HoldResourceReference(MeshBuffer);
        
        return true;
	}

    
	bool FRHIMetal::UpdateHardwareResource(FTexturePtr Texture)
	{
        FTextureMetal * TexMetal = static_cast<FTextureMetal*>(Texture.get());
        if (TexMetal->Texture == nil)
        {
            const TTextureDesc& Desc = Texture->GetDesc();
            
            MTLPixelFormat MtlFormat = GetMetalPixelFormat(Desc.Format);
            // Only support texture 2d for now
            TI_ASSERT(Desc.Type == ETT_TEXTURE_2D);
            TI_ASSERT(Desc.Mips == 1);
            
            MTLTextureDescriptor * TextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MtlFormat width:Desc.Width height:Desc.Height mipmapped:NO];
            TexMetal->Texture = [MtlDevice newTextureWithDescriptor:TextureDesc];
        }
        
        HoldResourceReference(Texture);
        
        return true;
	}

	bool FRHIMetal::UpdateHardwareResource(FTexturePtr Texture, TTexturePtr InTexData)
	{
        FTextureMetal * TexMetal = static_cast<FTextureMetal*>(Texture.get());
        TI_ASSERT(TexMetal->Texture == nil);
        
        const TTextureDesc& Desc = InTexData->GetDesc();
        
        MTLPixelFormat MtlFormat = GetMetalPixelFormat(Desc.Format);
        const bool IsCubeMap = Desc.Type == ETT_TEXTURE_CUBE;
        MTLTextureDescriptor * TextureDesc;
        if (IsCubeMap)
        {
            TI_ASSERT(Desc.Width == Desc.Height);
            TextureDesc = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:MtlFormat size:Desc.Width mipmapped:Desc.Mips > 1];
        }
        else
        {
            TextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MtlFormat width:Desc.Width height:Desc.Height mipmapped:Desc.Mips > 1];
        }
        TexMetal->Texture = [MtlDevice newTextureWithDescriptor:TextureDesc];
        
        int32 Faces = 1;
        if (IsCubeMap)
            Faces = 6;
        
        const TVector<TTexture::TSurface*>& TextureSurfaces = InTexData->GetSurfaces();
        for (int32 face = 0 ; face < Faces; ++ face)
        {
            int32 W = Desc.Width;
            int32 H = Desc.Height;
            for (int32 mip = 0; mip < Desc.Mips; ++ mip)
            {
                MTLRegion Region = MTLRegionMake2D(0, 0, W, H);
                const int32 SurfaceIndex = face * Desc.Mips + mip;
                const TTexture::TSurface* Surface = TextureSurfaces[SurfaceIndex];
                TI_ASSERT(Surface->RowPitch != 0);
                [TexMetal->Texture replaceRegion:Region mipmapLevel:mip slice:face withBytes:Surface->Data bytesPerRow:Surface->RowPitch bytesPerImage:0];
                W /= 2;
                H /= 2;
            }
        }

        HoldResourceReference(Texture);
        
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
        
        FShaderPtr Shader = InPipelineDesc->GetDesc().Shader->ShaderResource;
        FShaderMetal * ShaderMetal = static_cast<FShaderMetal*>(Shader.get());
        PipelineStateDesc.vertexFunction = ShaderMetal->VertexProgram;
        PipelineStateDesc.fragmentFunction = ShaderMetal->FragmentProgram;
        
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
            PipelineStateDesc.colorAttachments[r].pixelFormat = GetMetalPixelFormat(Desc.RTFormats[r]);
        }
        PipelineStateDesc.depthAttachmentPixelFormat = GetMetalPixelFormat(Desc.DepthFormat);
        PipelineStateDesc.stencilAttachmentPixelFormat = GetMetalPixelFormat(Desc.StencilFormat);
        
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
        
        if (InPipelineDesc->GetResourceName().find("SkinBase") != TString::npos) {
            NSLog(@"break here.");
        }
        
        // Create pso with reflection
        NSError* Err  = nil;
        MTLRenderPipelineReflection * ReflectionObj = nil;
        PipelineMetal->PipelineState = [MtlDevice newRenderPipelineStateWithDescriptor : PipelineStateDesc options:MTLPipelineOptionArgumentInfo reflection:&ReflectionObj error:&Err];
        
        // Create shader binding info for shaders
        TI_ASSERT(Shader->ShaderBinding == nullptr);
        Shader->ShaderBinding = ti_new FShaderBinding(0);   // Metal do not care NumBindingCount
        for (int32 i = 0 ; i < ReflectionObj.vertexArguments.count; ++i)
        {
            MTLArgument * Arg = ReflectionObj.vertexArguments[i];
            TString BindName = [Arg.name UTF8String];
            int32 BindIndex = (int32)Arg.index;
            if (BindName.substr(0, 12) == "vertexBuffer")
            {
                continue;
            }
            if (BindIndex >= 0)
            {
                E_ARGUMENT_TYPE ArgumentType = FShaderBinding::GetArgumentTypeByName(BindName, Arg.type == MTLArgumentTypeTexture);
                Shader->ShaderBinding->AddShaderArgument(ESS_VERTEX_SHADER,
                                                         FShaderBinding::FShaderArgument(BindIndex, ArgumentType, (int32)Arg.bufferDataSize));
            }
        }
        for (int32 i = 0 ; i < ReflectionObj.fragmentArguments.count; ++ i)
        {
            MTLArgument * Arg = ReflectionObj.fragmentArguments[i];
            TString BindName = [Arg.name UTF8String];
            int32 BindIndex = (int32)Arg.index;
            if (BindIndex >= 0)
            {
                E_ARGUMENT_TYPE ArgumentType = FShaderBinding::GetArgumentTypeByName(BindName, Arg.type == MTLArgumentTypeTexture);
                Shader->ShaderBinding->AddShaderArgument(ESS_PIXEL_SHADER,
                                                         FShaderBinding::FShaderArgument(BindIndex, ArgumentType, (int32)Arg.bufferDataSize));
            }
        }
        Shader->ShaderBinding->SortArguments();
        
        // Create depth stencil state
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
    
    bool FRHIMetal::UpdateHardwareResource(FRenderTargetPtr RenderTarget)
    {
        FRenderTargetMetal * RTMetal = static_cast<FRenderTargetMetal*>(RenderTarget.get());
        TI_ASSERT(RTMetal->RenderPassDesc == nil);
        RTMetal->RenderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        
        int32 ColorBufferCount = RenderTarget->GetColorBufferCount();
        for (int32 i = 0 ; i < ColorBufferCount ; ++ i)
        {
            const FRenderTarget::RTBuffer& ColorBuffer = RenderTarget->GetColorBuffer(i);
            MTLRenderPassColorAttachmentDescriptor * ColorAttachment = RTMetal->RenderPassDesc.colorAttachments[i];
            TI_ASSERT(ColorBuffer.Texture != nullptr);
            FTextureMetal * TextureMetal = static_cast<FTextureMetal*>(ColorBuffer.Texture.get());
            ColorAttachment.texture = TextureMetal->GetMetalTexture();
            ColorAttachment.loadAction = k_LOAD_ACTION_MAP[ColorBuffer.LoadAction];
            ColorAttachment.storeAction = k_STORE_ACTION_MAP[ColorBuffer.StoreAction];
            ColorAttachment.clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
        }
        
        // Depth stencil buffers
        {
            const FRenderTarget::RTBuffer& DepthStencilBuffer = RenderTarget->GetDepthStencilBuffer();
            FTexturePtr DSBufferTexture = DepthStencilBuffer.Texture;
            if (DSBufferTexture != nullptr)
            {
                MTLRenderPassDepthAttachmentDescriptor* DepthAttachment = RTMetal->RenderPassDesc.depthAttachment;
                FTextureMetal * TextureMetal = static_cast<FTextureMetal*>(DSBufferTexture.get());
                DepthAttachment.texture = TextureMetal->GetMetalTexture();
                DepthAttachment.clearDepth = 1.0;
                DepthAttachment.loadAction = k_LOAD_ACTION_MAP[DepthStencilBuffer.LoadAction];
                DepthAttachment.storeAction = k_STORE_ACTION_MAP[DepthStencilBuffer.StoreAction];
            }
        }
        return true;
    }
    
    bool FRHIMetal::UpdateHardwareResource(FShaderPtr ShaderResource)
    {
        // Load vertex function and pixel function
        FShaderMetal * ShaderMetal = static_cast<FShaderMetal*>(ShaderResource.get());
        TI_ASSERT(!ShaderResource->GetShaderName(ESS_VERTEX_SHADER).empty());
        NSString * VertexShader = [NSString stringWithUTF8String:ShaderResource->GetShaderName(ESS_VERTEX_SHADER).c_str()];
        NSString * FragmentShader = nil;
        if (!ShaderResource->GetShaderName(ESS_PIXEL_SHADER).empty())
        {
            FragmentShader = [NSString stringWithUTF8String:ShaderResource->GetShaderName(ESS_PIXEL_SHADER).c_str()];
        }
        
        ShaderMetal->VertexProgram = [DefaultLibrary newFunctionWithName:VertexShader];
        if(ShaderMetal->VertexProgram == nil)
        {
            _LOG(Fatal, "Can not load vertex function %s.\n", ShaderResource->GetShaderName(ESS_VERTEX_SHADER).c_str());
            return false;
        }
        
        if (FragmentShader != nil)
        {
            ShaderMetal->FragmentProgram = [DefaultLibrary newFunctionWithName:FragmentShader];
        }
        return true;
    }
    
    bool FRHIMetal::UpdateHardwareResource(FArgumentBufferPtr ArgumentBuffer, TStreamPtr ArgumentData, const TVector<FTexturePtr>& ArgumentTextures)
    {
        FArgumentBufferMetal * ArgMetal = static_cast<FArgumentBufferMetal*>(ArgumentBuffer.get());
        FShaderMetal * ShaderMetal = static_cast<FShaderMetal*>(ArgumentBuffer->GetShader().get());
        TI_ASSERT((ArgumentData != nullptr && ArgumentData->GetLength() > 0) || ArgumentTextures.size() > 0);
        
        // Create uniform data if exist
        id<MTLBuffer> UniformBuffer = nil;
        if (ArgumentData != nullptr && ArgumentData->GetLength() > 0)
        {
            UniformBuffer = [MtlDevice newBufferWithBytes:ArgumentData->GetBuffer() length:ArgumentData->GetLength() options:MTLResourceStorageModeShared];
        }
        
        // Create argument buffer, fill uniform and textures
        TI_ASSERT(ArgMetal->ArgumentBuffer == nil);
        id <MTLArgumentEncoder> argumentEncoder = [ShaderMetal->FragmentProgram newArgumentEncoderWithBufferIndex:0];
        NSUInteger argumentBufferLength = argumentEncoder.encodedLength;
        ArgMetal->ArgumentBuffer = [MtlDevice newBufferWithLength:argumentBufferLength options:0];
#if defined (TIX_DEBUG)
        const TString& ShaderName = ShaderMetal->GetShaderName(ESS_PIXEL_SHADER);
        TString ArgName = ShaderName + "_ArgumentBuffer";
        ArgMetal->ArgumentBuffer.label = [NSString stringWithUTF8String:ArgName.c_str()];
#endif
        [argumentEncoder setArgumentBuffer:ArgMetal->ArgumentBuffer offset:0];
        
        int32 ArgumentIndex = 0;
        // Fill uniform
        if (UniformBuffer != nil) {
            [argumentEncoder setBuffer:UniformBuffer offset:0 atIndex:ArgumentIndex];
            ++ ArgumentIndex;
        }
        
        // Fill textures
        for (const auto& Tex : ArgumentTextures) {
            FTextureMetal * TexMetal = static_cast<FTextureMetal*>(Tex.get());
            [argumentEncoder setTexture:TexMetal->Texture atIndex:ArgumentIndex];
            ++ ArgumentIndex;
        }
        
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
    
    void FRHIMetal::SetArgumentBuffer(FArgumentBufferPtr InArgumentBuffer)
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
    
    void FRHIMetal::HoldResourceReference(FRenderResourcePtr InResource)
    {
        FrameResources[CurrentFrame]->HoldReference(InResource);
    }
}
#endif	// COMPILE_WITH_RHI_METAL
