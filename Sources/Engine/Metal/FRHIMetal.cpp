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
#include "FUniformBufferMetal.h"

namespace tix
{
	FRHIMetal::FRHIMetal()
		: FRHI(ERHI_METAL)
        , CurrentFrame(0)
	{
        MtlLayer = nil;
        MtlDevice = nil;
        CurrentDrawable = nil;
        
        CommandQueue = nil;
        DefaultLibrary = nil;
        
        CommandBuffer = nil;
        RenderEncoder = nil;
        
        FrameBufferPassDesc = nil;
        
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
        
        // Grab a metal layer
        MtlLayer = MetalView.MtlLayer;
        TEngine::AppInfo.Width = (int32)(MtlLayer.bounds.size.width * TEngine::AppInfo.ContentScale);
        TEngine::AppInfo.Height = (int32)(MtlLayer.bounds.size.height * TEngine::AppInfo.ContentScale);
        
        // Grab a metal device
        MtlDevice = MTLCreateSystemDefaultDevice();
        MtlLayer.device = MtlDevice;
        
        // Create Command Queue
        CommandQueue = [MtlDevice newCommandQueue];
        
        // Create Frame buffer pass descriptor
        TI_ASSERT(FrameBufferPassDesc == nil);
        FrameBufferPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        
        // Create Metal Default Library
        NSFileManager* fm = [NSFileManager defaultManager];
        NSString* curr_path = [NSString stringWithString: fm.currentDirectoryPath];
        [fm changeCurrentDirectoryPath:@"/"];
        DefaultLibrary = [MtlDevice newDefaultLibrary];
        [fm changeCurrentDirectoryPath:curr_path];
        
        InflightSemaphore = dispatch_semaphore_create(FRHIConfig::FrameBufferNum);
        
        Viewport.Width = TEngine::AppInfo.Width;
        Viewport.Height = TEngine::AppInfo.Height;
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
    
    void FRHIMetal::BeginRenderToFrameBuffer()
    {
        CurrentDrawable = MtlLayer.nextDrawable;
        TI_ASSERT(CurrentDrawable != nil);
        id <MTLTexture> Texture = CurrentDrawable.texture;
        
        // create a color attachment every frame since we have to recreate the texture every frame
        MTLRenderPassColorAttachmentDescriptor * ColorAttachment = FrameBufferPassDesc.colorAttachments[0];
        ColorAttachment.texture = Texture;
        
        // MTLLoadActionDontCare every frame for best performance
        ColorAttachment.loadAction  = MTLLoadActionClear;//MTLLoadActionDontCare;
        ColorAttachment.clearColor  = MTLClearColorMake(1.0, 0.0, 1.0, 1.0);;
        
        // store only attachments that will be presented to the screen, as in this case
        ColorAttachment.storeAction = MTLStoreActionStore;
        
        RenderEncoder = [CommandBuffer renderCommandEncoderWithDescriptor:FrameBufferPassDesc];
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
        return ti_new FUniformBufferMetal(InStructSize);
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
            TextureDesc.usage |= MTLTextureUsageRenderTarget;
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
        if (Desc.SRGB)
        {
            MtlFormat = GetSRGBFormat(MtlFormat);
        }
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
        Pipeline->SetDesc(Desc);
        
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
                                                         FShaderBinding::FShaderArgument(BindIndex, ArgumentType));
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
                                                         FShaderBinding::FShaderArgument(BindIndex, ArgumentType));
            }
        }
        Shader->ShaderBinding->SortArguments();
        
        // Create depth stencil state
        MTLDepthStencilDescriptor * DepthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
        DepthStateDesc.depthCompareFunction = Desc.IsEnabled(EPSO_DEPTH_TEST) ? k_COMPARE_FUNC_MAP[Desc.DepthStencilDesc.DepthFunc] : MTLCompareFunctionAlways;
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

	static const int32 UniformBufferAlignSize = 16;
	bool FRHIMetal::UpdateHardwareResource(FUniformBufferPtr UniformBuffer, void* InData)
	{
        FUniformBufferMetal * UBMetal = static_cast<FUniformBufferMetal*>(UniformBuffer.get());
        
        const int32 AlignedDataSize = ti_align(UniformBuffer->GetStructSize(), UniformBufferAlignSize);
        TI_ASSERT(UBMetal->ConstantBuffer == nil);
        UBMetal->ConstantBuffer = [MtlDevice newBufferWithBytes:InData length:AlignedDataSize options:MTLResourceStorageModeShared];
        HoldResourceReference(UniformBuffer);
        
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
            ColorAttachment.texture = TextureMetal->Texture;
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
                DepthAttachment.texture = TextureMetal->Texture;
                DepthAttachment.clearDepth = 1.0;
                DepthAttachment.loadAction = k_LOAD_ACTION_MAP[DepthStencilBuffer.LoadAction];
                DepthAttachment.storeAction = k_STORE_ACTION_MAP[DepthStencilBuffer.StoreAction];
                
                // Only support depth_stencil in one texture for now. Do not support seperate stencil texture.
                TI_ASSERT(DSBufferTexture->GetDesc().Format == EPF_DEPTH24_STENCIL8);
                MTLRenderPassStencilAttachmentDescriptor* StencilAttachment = RTMetal->RenderPassDesc.stencilAttachment;
                StencilAttachment.texture = TextureMetal->Texture;
                StencilAttachment.clearStencil = 0;
                StencilAttachment.loadAction = k_LOAD_ACTION_MAP[DepthStencilBuffer.LoadAction];
                StencilAttachment.storeAction = k_STORE_ACTION_MAP[DepthStencilBuffer.StoreAction];
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
        FArgumentBufferMetal * ArgBufferMetal = static_cast<FArgumentBufferMetal*>(ArgumentBuffer.get());
        TI_ASSERT(ArgBufferMetal->ArgumentBindIndex < 0 && ArgBufferMetal->Buffers.size() == 0 && ArgBufferMetal->Textures.size() == 0);
        FShaderPtr Shader = ArgumentBuffer->GetShader();
        FShaderMetal * ShaderMetal = static_cast<FShaderMetal*>(ArgumentBuffer->GetShader().get());
        TI_ASSERT((ArgumentData != nullptr && ArgumentData->GetLength() > 0) || ArgumentTextures.size() > 0);
        
        // Get Uniform buffer Bind Index
        FShaderBindingPtr ShaderBinding = Shader->ShaderBinding;
        ArgBufferMetal->ArgumentBindIndex = ShaderBinding->GetFirstPSBindingIndexByType(ARGUMENT_MI_BUFFER);
        TI_ASSERT(ArgBufferMetal->ArgumentBindIndex >= 0);
        
        // Create uniform data if exist
        id<MTLBuffer> UniformBuffer = nil;
        if (ArgumentData != nullptr && ArgumentData->GetLength() > 0)
        {
            UniformBuffer = [MtlDevice newBufferWithBytes:ArgumentData->GetBuffer() length:ArgumentData->GetLength() options:MTLResourceStorageModeShared];
        }
        
        // Create argument buffer, fill uniform and textures
        TI_ASSERT(ArgBufferMetal->ArgumentBuffer == nil);
        id <MTLArgumentEncoder> argumentEncoder = [ShaderMetal->FragmentProgram newArgumentEncoderWithBufferIndex:ArgBufferMetal->ArgumentBindIndex];
        NSUInteger argumentBufferLength = argumentEncoder.encodedLength;
        ArgBufferMetal->ArgumentBuffer = [MtlDevice newBufferWithLength:argumentBufferLength options:0];
#if defined (TIX_DEBUG)
        const TString& ShaderName = ShaderMetal->GetShaderName(ESS_PIXEL_SHADER);
        TString ArgName = ShaderName + "_ArgumentBuffer";
        ArgBufferMetal->ArgumentBuffer.label = [NSString stringWithUTF8String:ArgName.c_str()];
#endif
        [argumentEncoder setArgumentBuffer:ArgBufferMetal->ArgumentBuffer offset:0];
        
        int32 ArgumentIndex = 0;
        // Fill uniform
        if (UniformBuffer != nil) {
            [argumentEncoder setBuffer:UniformBuffer offset:0 atIndex:ArgumentIndex];
            ++ ArgumentIndex;
            ArgBufferMetal->Buffers.push_back(UniformBuffer);
        }
        
        // Fill textures
        for (const auto& Tex : ArgumentTextures) {
            FTextureMetal * TexMetal = static_cast<FTextureMetal*>(Tex.get());
            [argumentEncoder setTexture:TexMetal->Texture atIndex:ArgumentIndex];
            ++ ArgumentIndex;
            ArgBufferMetal->Textures.push_back(TexMetal->Texture);
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
        TI_TODO("Don't set pipeline duplicated.");
        FPipelineMetal* PLMetal = static_cast<FPipelineMetal*>(InPipeline.get());
        
        [RenderEncoder setRenderPipelineState:PLMetal->PipelineState];
        [RenderEncoder setDepthStencilState:PLMetal->DepthState];
        
        E_CULL_MODE Cull = (E_CULL_MODE)InPipeline->GetDesc().RasterizerDesc.CullMode;
        [RenderEncoder setFrontFacingWinding:MTLWindingClockwise];
        [RenderEncoder setCullMode:k_CULL_MODE_MAP[Cull]];
	}

	void FRHIMetal::SetMeshBuffer(FMeshBufferPtr InMeshBuffer)
    {
        FMeshBufferMetal* MBMetal = static_cast<FMeshBufferMetal*>(InMeshBuffer.get());
        TI_ASSERT(RenderEncoder != nil);
        [RenderEncoder setVertexBuffer:MBMetal->VertexBuffer offset:0 atIndex:0];
	}

	void FRHIMetal::SetUniformBuffer(E_SHADER_STAGE ShaderStage, int32 BindIndex, FUniformBufferPtr InUniformBuffer)
	{
        FUniformBufferMetal * UBMetal = static_cast<FUniformBufferMetal*>(InUniformBuffer.get());
        if (ShaderStage == ESS_VERTEX_SHADER) {
            [RenderEncoder setVertexBuffer:UBMetal->ConstantBuffer offset:0 atIndex:BindIndex];
        }
        else if (ShaderStage == ESS_PIXEL_SHADER) {
            [RenderEncoder setFragmentBuffer:UBMetal->ConstantBuffer offset:0 atIndex:BindIndex];
        }
        else {
            TI_ASSERT(0);
        }
	}

	void FRHIMetal::SetRenderResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable)
	{
        TI_ASSERT(0);
	}

	void FRHIMetal::SetShaderTexture(int32 BindIndex, FTexturePtr InTexture)
	{
        FTextureMetal * TexMetal = static_cast<FTextureMetal*>(InTexture.get());
        [RenderEncoder setFragmentTexture:TexMetal->Texture atIndex:BindIndex];
	}
    
    void FRHIMetal::SetArgumentBuffer(FArgumentBufferPtr InArgumentBuffer)
    {
        FArgumentBufferMetal * ABMetal = static_cast<FArgumentBufferMetal*>(InArgumentBuffer.get());
        TI_ASSERT(ABMetal->ArgumentBindIndex >= 0 && ABMetal->ArgumentBindIndex < 31);
        
        // Indicate buffers and textures usage
        for (int32 i = 0; i < (int32)ABMetal->Buffers.size(); ++ i) {
            [RenderEncoder useResource:ABMetal->Buffers[i] usage:MTLResourceUsageRead];
        }
        for (int32 i = 0; i < (int32)ABMetal->Textures.size(); ++ i) {
            [RenderEncoder useResource:ABMetal->Textures[i] usage:MTLResourceUsageSample];
        }
        
        // Set argument buffer
        [RenderEncoder setFragmentBuffer:ABMetal->ArgumentBuffer
                                  offset:0
                                 atIndex:ABMetal->ArgumentBindIndex];
    }

	void FRHIMetal::SetStencilRef(uint32 InRefValue)
	{
        if (RenderEncoder != nil) {
            [RenderEncoder setStencilReferenceValue:InRefValue];
        }
	}

	void FRHIMetal::DrawPrimitiveIndexedInstanced(FMeshBufferPtr MeshBuffer, uint32 InstanceCount)
    {
        FMeshBufferMetal * MBMetal = static_cast<FMeshBufferMetal*>(MeshBuffer.get());
        
        [RenderEncoder drawIndexedPrimitives:k_PRIMITIVE_TYPE_MAP[MeshBuffer->GetPrimitiveType()]
                                  indexCount:MeshBuffer->GetIndicesCount()
                                   indexType:k_INDEX_TYPE_MAP[MeshBuffer->GetIndexType()]
                                 indexBuffer:MBMetal->IndexBuffer
                           indexBufferOffset:0];
	}

	void FRHIMetal::SetViewport(const FViewport& VP)
	{
        FRHI::SetViewport(VP);
        
        MTLViewport vp;
        vp.originX = VP.Left;
        vp.originY = VP.Top;
        vp.width = VP.Width;
        vp.height = VP.Height;
        vp.znear = 0.0;
        vp.zfar = 1.0;
        if (RenderEncoder != nil)
        {
            [RenderEncoder setViewport:vp];
        }
	}

	void FRHIMetal::PushRenderTarget(FRenderTargetPtr RT, const int8* PassName)
    {
        FRenderTargetMetal * RTMetal = static_cast<FRenderTargetMetal*>(RT.get());
        RenderEncoder = [CommandBuffer renderCommandEncoderWithDescriptor:RTMetal->RenderPassDesc];
#if defined (TIX_DEBUG)
        RenderEncoder.label = [NSString stringWithUTF8String:PassName];
#endif
        
		FRHI::PushRenderTarget(RT, PassName);
        
        // Set scissor rect
        const FViewport& VP = RtViewports.back();
        MTLScissorRect Rect;
        Rect.x = 0;
        Rect.y = 0;
        Rect.width = VP.Width;
        Rect.height = VP.Height;
        [RenderEncoder setScissorRect:Rect];
        
        // Try cull
        //[RenderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
        //[RenderEncoder setCullMode:MTLCullModeBack];
	}

	FRenderTargetPtr FRHIMetal::PopRenderTarget()
    {
        TI_ASSERT(RenderTargets.size() > 0);
        
        RenderTargets.pop_back();
        RtViewports.pop_back();
        
        FRenderTargetPtr RT  = nullptr;
        if (RenderTargets.size() != 0)
            RT = RenderTargets.back();
        
        if (RT != nullptr)
        {
            TI_ASSERT(0);
            [RenderEncoder endEncoding];
            //FRenderTargetMetal * RTMetal = static_cast<FRenderTargetMetal*>(RT.get());
            //SetupPipeline(rtMetal->_colorBuffer, rtMetal->_depthBuffer, MTLClearColorMake(0.f, 0.f, 0.f, 0.f));
            //RenderEncoder = [_commandBuffer renderCommandEncoderWithDescriptor:_renderPassDescriptor];
        }
        else
        {
            [RenderEncoder endEncoding];
        }
        RenderEncoder = nil;
        
        return RT;
	}
    
    void FRHIMetal::HoldResourceReference(FRenderResourcePtr InResource)
    {
        FrameResources[CurrentFrame]->HoldReference(InResource);
    }
}
#endif	// COMPILE_WITH_RHI_METAL
