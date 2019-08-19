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
#include "FFrameResourcesMetal.h"
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
        , LastFrameMark(0)
        , CurrentFrame(0)
	{
        MtlLayer = nil;
        MtlDevice = nil;
        CurrentDrawable = nil;
        
        CommandQueue = nil;
        DefaultLibrary = nil;
        
        CommandBuffer = nil;
        
        RenderEncoder = nil;
        ComputeEncoder = nil;
        BlitEncoder = nil;
        
        FrameBufferPassDesc = nil;
        
        // Create frame resource holders
        for (int32 i = 0 ; i < FRHIConfig::FrameBufferNum; ++ i)
        {
            ResHolders[i] = ti_new FFrameResourcesMetal;
            FrameResources[i] = ResHolders[i];
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
        FRHI::BeginFrame();
        
        TI_ASSERT(CommandBuffer == nil);
        TI_ASSERT(RenderEncoder == nil);
        TI_ASSERT(ComputeEncoder == nil);
        TI_ASSERT(BlitEncoder == nil);
        
        dispatch_semaphore_wait(InflightSemaphore, DISPATCH_TIME_FOREVER);
        FrameResources[LastFrameMark]->RemoveAllReferences();
        
        CommandBuffer = [CommandQueue commandBuffer];
        CommandBuffer.label = @"Render Command Buffer";
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
            LastFrameMark = CurrentFrame;
            CurrentFrame = (CurrentFrame + 1) % FRHIConfig::FrameBufferNum;

            // Remember GPU Frames 
            FRHI::GPUFrameDone();
        }];
        
        [CommandBuffer presentDrawable:CurrentDrawable];
        [CommandBuffer commit];
        
        RenderEncoder = nil;
        ComputeEncoder = nil;
        BlitEncoder = nil;
        
        CommandBuffer = nil;
        CurrentDrawable = nil;
	}
    
    void FRHIMetal::BeginRenderToFrameBuffer()
    {
        CloseCurrentEncoderIfNotMatch(EPL_INVALID);
        
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
        CurrentCommandListState.ListType = EPL_GRAPHICS;
    }
    
    void FRHIMetal::BeginComputeTask()
    {
        // Switch from graphics command list to compute command list.
        bool EncoderClosed = CloseCurrentEncoderIfNotMatch(EPL_COMPUTE);
        
        if (EncoderClosed)
        {
            // Create compute encoder
            CurrentCommandListCounter[EPL_COMPUTE] ++;
            TI_ASSERT(ComputeEncoder == nil);
            ComputeEncoder = [CommandBuffer computeCommandEncoder];
            
            // Remember the list we are using, and push it to order vector
            CurrentCommandListState.ListType = EPL_COMPUTE;
            CurrentCommandListState.ListIndex = CurrentCommandListCounter[EPL_COMPUTE];
            ListExecuteOrder.push_back(CurrentCommandListState);
        }
        else
        {
            TI_ASSERT(ComputeEncoder != nil);
        }
    }
    
    void FRHIMetal::EndComputeTask()
    {
        TI_ASSERT(ComputeEncoder != nil);
        // Do not close compute encoder here. Close it in CloseCurrentEncoder.
        //[ComputeEncoder endEncoding];
        //ComputeEncoder = nil;
    }
    
    bool FRHIMetal::CloseCurrentEncoderIfNotMatch(E_PIPELINE_TYPE NextPipelineType)
    {
        if (CurrentCommandListState.ListType == NextPipelineType)
        {
            // Current Encoder match next encoder
            return false;
        }
        if (CurrentCommandListState.ListType == EPL_GRAPHICS)
        {
            TI_ASSERT(RenderEncoder != nil);
            [RenderEncoder endEncoding];
            RenderEncoder = nil;
        }
        else if (CurrentCommandListState.ListType == EPL_COMPUTE)
        {
            TI_ASSERT(ComputeEncoder != nil);
            [ComputeEncoder endEncoding];
            ComputeEncoder = nil;
        }
        else if (CurrentCommandListState.ListType == EPL_BLIT)
        {
            TI_ASSERT(BlitEncoder != nil);
            [BlitEncoder endEncoding];
            BlitEncoder = nil;
        }
        CurrentCommandListState.ListType = EPL_INVALID;
        return true;
    }
    
    id <MTLBlitCommandEncoder> FRHIMetal::RequestBlitEncoder()
    {
        // Close other encoders
        bool EncoderClosed = CloseCurrentEncoderIfNotMatch(EPL_BLIT);
        if (EncoderClosed)
        {
            // Create a new BlitEncoder
            BlitEncoder = [CommandBuffer blitCommandEncoder];
            CurrentCommandListState.ListType = EPL_BLIT;
        }
        else
        {
            TI_ASSERT(BlitEncoder != nil);
        }
        return BlitEncoder;
    }
    
	FTexturePtr FRHIMetal::CreateTexture()
	{
        return ti_new FTextureMetal;
	}

	FTexturePtr FRHIMetal::CreateTexture(const TTextureDesc& Desc)
	{
        return ti_new FTextureMetal(Desc);
	}

	FUniformBufferPtr FRHIMetal::CreateUniformBuffer(uint32 InStructSizeInBytes, uint32 Elements, uint32 Flag)
	{
        return ti_new FUniformBufferMetal(InStructSizeInBytes, Elements, Flag);
	}

	FMeshBufferPtr FRHIMetal::CreateMeshBuffer()
	{
        return ti_new FMeshBufferMetal;
	}
    
    FInstanceBufferPtr FRHIMetal::CreateInstanceBuffer()
    {
        return ti_new FInstanceBufferMetal;
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
    
    FShaderPtr FRHIMetal::CreateComputeShader(const TString& InComputeShaderName)
    {
        return ti_new FShaderMetal(InComputeShaderName);
    }
    
    FArgumentBufferPtr FRHIMetal::CreateArgumentBuffer(int32 ReservedSlots)
    {
        return ti_new FArgumentBufferMetal(ReservedSlots);
    }
    
    int32 FRHIMetal::GetCurrentEncodingFrameIndex()
    {
        TI_ASSERT(0);
        return 0;
    }

	// Wait for pending GPU work to complete.
	void FRHIMetal::WaitingForGpu()
	{
        TI_ASSERT(0);
	}

	bool FRHIMetal::UpdateHardwareResourceMesh(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData)
	{
#if defined (TIX_DEBUG)
        MeshBuffer->SetResourceName(InMeshData->GetResourceName());
#endif
        FMeshBufferMetal * MBMetal = static_cast<FMeshBufferMetal*>(MeshBuffer.get());
        
        TI_TODO("Create mesh buffer as GPU access only.");
        // Create Vertex Buffer
        const int32 BufferSize = InMeshData->GetVerticesCount() * InMeshData->GetStride();
        MBMetal->VertexBuffer = [MtlDevice newBufferWithBytes:InMeshData->GetVSData() length:BufferSize options:MTLResourceCPUCacheModeDefaultCache];
        
        const uint32 IndexBufferSize = (InMeshData->GetIndicesCount() * (InMeshData->GetIndexType() == EIT_16BIT ? sizeof(uint16) : sizeof(uint32)));
        MBMetal->IndexBuffer = [MtlDevice newBufferWithBytes:InMeshData->GetPSData() length:IndexBufferSize options:MTLResourceCPUCacheModeDefaultCache];
        
        // Hold resources used here
        HoldResourceReference(MeshBuffer);
        
        return true;
	}

    bool FRHIMetal::UpdateHardwareResourceIB(FInstanceBufferPtr InstanceBuffer, TInstanceBufferPtr InInstanceData)
    {
#if defined (TIX_DEBUG)
        InstanceBuffer->SetResourceName(InInstanceData->GetResourceName());
#endif
        FInstanceBufferMetal * IBMetal = static_cast<FInstanceBufferMetal*>(InstanceBuffer.get());
        
        TI_TODO("Create instance buffer as GPU access only.");
        // Create Instance Buffer
        const int32 BufferSize = InInstanceData->GetInstanceCount() * InInstanceData->GetStride();
        IBMetal->InstanceBuffer = [MtlDevice newBufferWithBytes:InInstanceData->GetInstanceData() length:BufferSize options:MTLResourceCPUCacheModeDefaultCache];
        
        // Hold resources used here
        HoldResourceReference(InstanceBuffer);
        
        return true;
    }
    
	bool FRHIMetal::UpdateHardwareResourceTexture(FTexturePtr Texture)
	{
        FTextureMetal * TexMetal = static_cast<FTextureMetal*>(Texture.get());
        if (TexMetal->Texture == nil)
        {
            const TTextureDesc& Desc = Texture->GetDesc();
            
            MTLPixelFormat MtlFormat = GetMetalPixelFormat(Desc.Format);
            // Only support texture 2d for now
            TI_ASSERT(Desc.Type == ETT_TEXTURE_2D);
            
            // Follow the instructions here to create a private MTLBuffer
            // https://developer.apple.com/documentation/metal/setting_resource_storage_modes/choosing_a_resource_storage_mode_in_ios_and_tvos?language=objc
            MTLTextureDescriptor * TextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MtlFormat width:Desc.Width height:Desc.Height mipmapped:Desc.Mips > 1];
            TextureDesc.mipmapLevelCount = Desc.Mips;
            TextureDesc.storageMode = MTLStorageModePrivate;
            if ((Desc.Flags & ETF_RT_COLORBUFFER) != 0 ||
                (Desc.Flags & ETF_RT_DSBUFFER) != 0)
            {
                TI_ASSERT(Desc.Mips == 1);
                TextureDesc.usage |= MTLTextureUsageRenderTarget;
            }
            TexMetal->Texture = [MtlDevice newTextureWithDescriptor:TextureDesc];
        }
        
        HoldResourceReference(Texture);
        
        return true;
	}

	bool FRHIMetal::UpdateHardwareResourceTexture(FTexturePtr Texture, TTexturePtr InTexData)
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
        TI_TODO("Create normal texture as private");
        // Follow the instructions here to create a private MTLBuffer
        // https://developer.apple.com/documentation/metal/setting_resource_storage_modes/choosing_a_resource_storage_mode_in_ios_and_tvos?language=objc
        TextureDesc.mipmapLevelCount = Desc.Mips;
        TextureDesc.storageMode = MTLStorageModeShared;
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
    
    bool FRHIMetal::UpdateHardwareResourceTexture(FTexturePtr Texture, TImagePtr InImageData)
    {
        FTextureMetal * TexMetal = static_cast<FTextureMetal*>(Texture.get());
        const TTextureDesc& Desc = Texture->GetDesc();
        TI_ASSERT(Desc.Width == InImageData->GetWidth() && Desc.Height == InImageData->GetHeight());
        TI_ASSERT(Desc.Mips == InImageData->GetMipmapCount());
        
        if (TexMetal->Texture == nil)
        {
            MTLPixelFormat MtlFormat = GetMetalPixelFormat(Desc.Format);
            const bool IsCubeMap = Desc.Type == ETT_TEXTURE_CUBE;
            TI_ASSERT(!IsCubeMap);
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
            TI_TODO("Create normal texture as private");
            // Follow the instructions here to create a private MTLBuffer
            // https://developer.apple.com/documentation/metal/setting_resource_storage_modes/choosing_a_resource_storage_mode_in_ios_and_tvos?language=objc
            TextureDesc.mipmapLevelCount = Desc.Mips;
            TextureDesc.storageMode = MTLStorageModeShared;
            TexMetal->Texture = [MtlDevice newTextureWithDescriptor:TextureDesc];
        }
        
        const int32 Mips = Desc.Mips;
        int32 W = Desc.Width;
        int32 H = Desc.Height;
        for (int32 M = 0 ; M < Mips ; ++ M)
        {
            const TImage::TSurfaceData& MipSurface = InImageData->GetMipmap(M);
            MTLRegion Region = MTLRegionMake2D(0, 0, W, H);
            [TexMetal->Texture replaceRegion:Region mipmapLevel:M slice:0 withBytes:MipSurface.Data.GetBuffer() bytesPerRow:MipSurface.RowPitch bytesPerImage:0];
            W /= 2;
            H /= 2;
        }
        
        HoldResourceReference(Texture);
        return true;
    }

	bool FRHIMetal::UpdateHardwareResourcePL(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc)
    {
        FPipelineMetal * PipelineMetal = static_cast<FPipelineMetal*>(Pipeline.get());
        FShaderPtr Shader = Pipeline->GetShader();
        
        if (Shader->GetShaderType() == EST_RENDER)
        {
            TI_ASSERT(InPipelineDesc != nullptr);
            const TPipelineDesc& Desc = InPipelineDesc->GetDesc();
            TI_ASSERT(Shader == Desc.Shader->ShaderResource);
            
            MTLRenderPipelineDescriptor * PipelineStateDesc = [[MTLRenderPipelineDescriptor alloc] init];
#if defined (TIX_DEBUG)
            Pipeline->SetResourceName(InPipelineDesc->GetResourceName());
            NSString * PipelineName = [NSString stringWithUTF8String:InPipelineDesc->GetResourceName().c_str()];
            PipelineStateDesc.label = PipelineName;
#endif
            PipelineStateDesc.sampleCount = 1;
            
            FShaderPtr Shader = InPipelineDesc->GetDesc().Shader->ShaderResource;
            FShaderMetal * ShaderMetal = static_cast<FShaderMetal*>(Shader.get());
            PipelineStateDesc.vertexFunction = ShaderMetal->VertexComputeProgram;
            PipelineStateDesc.fragmentFunction = ShaderMetal->FragmentProgram;

            // Set vertex layout
            TVector<E_MESH_STREAM_INDEX> VertexStreams = TMeshBuffer::GetSteamsFromFormat(Desc.VsFormat);
            MTLVertexDescriptor * VertexDesc = [[MTLVertexDescriptor alloc] init];
            
            uint32 VertexDataOffset = 0;
            for (uint32 i = 0; i < (uint32)VertexStreams.size(); ++i)
            {
                E_MESH_STREAM_INDEX Stream = VertexStreams[i];
                VertexDesc.attributes[i].format = k_MESHBUFFER_STREAM_FORMAT_MAP[Stream];
                VertexDesc.attributes[i].bufferIndex = 0;
                VertexDesc.attributes[i].offset = VertexDataOffset;
                VertexDataOffset += TMeshBuffer::SemanticSize[Stream];
            }

            // Set instance layout
            TVector<E_INSTANCE_STREAM_INDEX> InstanceStreams = TInstanceBuffer::GetSteamsFromFormat(Desc.InsFormat);
            uint32 InstanceDataOffset = 0;
            for (uint32 i = 0; i < InstanceStreams.size(); ++i)
            {
                E_INSTANCE_STREAM_INDEX Stream = InstanceStreams[i];
                VertexDesc.attributes[i + VertexStreams.size()].format = k_INSTANCEBUFFER_STREAM_FORMAT_MAP[Stream];
                VertexDesc.attributes[i + VertexStreams.size()].bufferIndex = 1;
                VertexDesc.attributes[i + VertexStreams.size()].offset = InstanceDataOffset;
                InstanceDataOffset += TInstanceBuffer::SemanticSize[Stream];
            }
            
            VertexDesc.layouts[0].stride = TMeshBuffer::GetStrideFromFormat(Desc.VsFormat);
            VertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
            VertexDesc.layouts[1].stride = TInstanceBuffer::GetStrideFromFormat(Desc.InsFormat);
            VertexDesc.layouts[1].stepFunction = MTLVertexStepFunctionPerInstance;
            VertexDesc.layouts[1].stepRate = 1;
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
            PipelineMetal->RenderPipelineState = [MtlDevice newRenderPipelineStateWithDescriptor : PipelineStateDesc options:MTLPipelineOptionArgumentInfo reflection:&ReflectionObj error:&Err];
            
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
                    E_ARGUMENT_TYPE ArgumentType = FShaderBinding::GetArgumentTypeByName(BindName);
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
                    E_ARGUMENT_TYPE ArgumentType = FShaderBinding::GetArgumentTypeByName(BindName);
                    Shader->ShaderBinding->AddShaderArgument(ESS_PIXEL_SHADER,
                                                             FShaderBinding::FShaderArgument(BindIndex, ArgumentType));
                }
            }
            Shader->ShaderBinding->PostInitArguments();
            
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
        }
        else
        {
            TI_ASSERT(Shader != nullptr);
            FShaderMetal * ShaderMetal = static_cast<FShaderMetal*>(Shader.get());
            
            // Create pso with reflection
            NSError* Err  = nil;
            MTLComputePipelineReflection * ReflectionObj = nil;
            PipelineMetal->ComputePipelineState = [MtlDevice newComputePipelineStateWithFunction : ShaderMetal->VertexComputeProgram options: MTLPipelineOptionArgumentInfo reflection:&ReflectionObj error:&Err];
            
            // Create shader binding info for shaders
            TI_ASSERT(Shader->ShaderBinding == nullptr);
            Shader->ShaderBinding = ti_new FShaderBinding(0);   // Metal do not care NumBindingCount
            for (int32 i = 0; i < ReflectionObj.arguments.count; ++ i)
            {
                MTLArgument * Arg = ReflectionObj.arguments[i];
                TString BindName = [Arg.name UTF8String];
                int32 BindIndex = (int32)Arg.index;
                if (BindIndex >= 0)
                {
                    E_ARGUMENT_TYPE ArgumentType = FShaderBinding::GetArgumentTypeByName(BindName);
                    Shader->ShaderBinding->AddShaderArgument(ESS_COMPUTE_SHADER,
                                                             FShaderBinding::FShaderArgument(BindIndex, ArgumentType));
                }
            }
            Shader->ShaderBinding->PostInitArguments();
        }
        HoldResourceReference(Pipeline);
        
		return true;
	}

	static const int32 UniformBufferAlignSize = 16;
	bool FRHIMetal::UpdateHardwareResourceUB(FUniformBufferPtr UniformBuffer, const void* InData)
	{
        FUniformBufferMetal * UBMetal = static_cast<FUniformBufferMetal*>(UniformBuffer.get());
        
        const int32 AlignedDataSize = ti_align(UniformBuffer->GetTotalBufferSize(), UniformBufferAlignSize);
        TI_ASSERT(UBMetal->Buffer == nil);
        
        if ((UniformBuffer->GetFlag() & UB_FLAG_GPU_ONLY) != 0)
        {
            UBMetal->Buffer = [MtlDevice newBufferWithLength:AlignedDataSize options:MTLStorageModePrivate];
        }
        else
        {
            UBMetal->Buffer = [MtlDevice newBufferWithLength:AlignedDataSize options:MTLStorageModeShared];
        }
        
        if (InData != nullptr)
        {
            // Upload data
            if ((UniformBuffer->GetFlag() & UB_FLAG_GPU_ONLY) != 0)
            {
                // Follow the instructions here to create a private MTLBuffer
                // https://developer.apple.com/documentation/metal/setting_resource_storage_modes/choosing_a_resource_storage_mode_in_ios_and_tvos?language=objc
                // Upload data to GPU only buffer
                id<MTLBuffer> UploadBuffer = [MtlDevice newBufferWithBytes: InData length: AlignedDataSize options:MTLStorageModeShared];
                id<MTLBlitCommandEncoder> BlitCommandEncoder = RequestBlitEncoder();
                [BlitCommandEncoder copyFromBuffer: UploadBuffer
                                      sourceOffset: 0
                                          toBuffer: UBMetal->Buffer
                                 destinationOffset: 0
                                              size: AlignedDataSize];
                HoldResourceReference(UploadBuffer);
            }
            else
            {
                memcpy([UBMetal->Buffer contents], InData, UniformBuffer->GetTotalBufferSize());
            }
        }
        
        HoldResourceReference(UniformBuffer);
        
        return true;
	}
    
    bool FRHIMetal::UpdateHardwareResourceRT(FRenderTargetPtr RenderTarget)
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
    
    bool FRHIMetal::UpdateHardwareResourceShader(FShaderPtr ShaderResource, TShaderPtr InShaderSource)
    {
        // Load vertex function and pixel function
        FShaderMetal * ShaderMetal = static_cast<FShaderMetal*>(ShaderResource.get());
        
        if (ShaderResource->GetShaderType() == EST_COMPUTE)
        {
            TI_ASSERT(!ShaderResource->GetShaderName(ESS_COMPUTE_SHADER).empty());
            NSString * ComputeShader = [NSString stringWithUTF8String:ShaderResource->GetShaderName(ESS_COMPUTE_SHADER).c_str()];
            
            ShaderMetal->VertexComputeProgram = [DefaultLibrary newFunctionWithName:ComputeShader];
            if(ShaderMetal->VertexComputeProgram == nil)
            {
                _LOG(Fatal, "Can not load compute function %s.\n", ShaderResource->GetShaderName(ESS_COMPUTE_SHADER).c_str());
                return false;
            }
        }
        else
        {
            TI_ASSERT(!ShaderResource->GetShaderName(ESS_VERTEX_SHADER).empty());
            NSString * VertexShader = [NSString stringWithUTF8String:ShaderResource->GetShaderName(ESS_VERTEX_SHADER).c_str()];
            NSString * FragmentShader = nil;
            if (!ShaderResource->GetShaderName(ESS_PIXEL_SHADER).empty())
            {
                FragmentShader = [NSString stringWithUTF8String:ShaderResource->GetShaderName(ESS_PIXEL_SHADER).c_str()];
            }
            
            ShaderMetal->VertexComputeProgram = [DefaultLibrary newFunctionWithName:VertexShader];
            if(ShaderMetal->VertexComputeProgram == nil)
            {
                _LOG(Fatal, "Can not load vertex function %s.\n", ShaderResource->GetShaderName(ESS_VERTEX_SHADER).c_str());
                return false;
            }
            
            if (FragmentShader != nil)
            {
                ShaderMetal->FragmentProgram = [DefaultLibrary newFunctionWithName:FragmentShader];
            }
        }
        
        // Metal shader binding created at Pipeline creation, since we need reflection object
        return true;
    }
    
    bool FRHIMetal::UpdateHardwareResourceAB(FArgumentBufferPtr ArgumentBuffer, FShaderPtr InShader, int32 SpecifiedBindingIndex)
    {
        FArgumentBufferMetal * ArgBufferMetal = static_cast<FArgumentBufferMetal*>(ArgumentBuffer.get());
        const TVector<FRenderResourcePtr>& Arguments = ArgumentBuffer->GetArguments();
        TI_ASSERT(ArgBufferMetal->ArgumentBuffer == nil && Arguments.size() > 0);
        
        FShaderMetal * ShaderMetal = static_cast<FShaderMetal*>(InShader.get());

        int32 ArgumentBindIndex = SpecifiedBindingIndex;
        id <MTLArgumentEncoder> ArgumentEncoder = nil;
        
#if defined (TIX_DEBUG)
        TString ShaderName;
#endif
        if (InShader->GetShaderType() == EST_RENDER)
        {
            TI_TODO("Support vertex argument buffer.");
            // Get Uniform buffer Bind Index
            if (ArgumentBindIndex < 0)
            {
                FShaderBindingPtr ShaderBinding = InShader->ShaderBinding;
                ArgumentBindIndex = ShaderBinding->GetPixelArgumentBufferBindingIndex();
            }
            ArgumentEncoder = [ShaderMetal->FragmentProgram newArgumentEncoderWithBufferIndex:ArgumentBindIndex];
#if defined (TIX_DEBUG)
            ShaderName = ShaderMetal->GetShaderName(ESS_PIXEL_SHADER);
#endif
        }
        else
        {
            // Compute shader
            if (ArgumentBindIndex < 0)
            {
                FShaderBindingPtr ShaderBinding = InShader->ShaderBinding;
                ArgumentBindIndex = ShaderBinding->GetVertexComputeArgumentBufferBindingIndex();
            }
            ArgumentEncoder = [ShaderMetal->VertexComputeProgram newArgumentEncoderWithBufferIndex:ArgumentBindIndex];
#if defined (TIX_DEBUG)
            ShaderName = ShaderMetal->GetShaderName(ESS_COMPUTE_SHADER);
#endif
        }
        TI_ASSERT(ArgumentBindIndex >= 0);
        TI_ASSERT(ArgumentEncoder != nil);
        
        // Create argument buffer, fill uniform and textures
        NSUInteger ArgumentBufferLength = ArgumentEncoder.encodedLength;
        ArgBufferMetal->ArgumentBuffer = [MtlDevice newBufferWithLength:ArgumentBufferLength options:0];
#if defined (TIX_DEBUG)
        TString ArgName = ShaderName + "_ArgumentBuffer";
        ArgBufferMetal->ArgumentBuffer.label = [NSString stringWithUTF8String:ArgName.c_str()];
#endif
        [ArgumentEncoder setArgumentBuffer:ArgBufferMetal->ArgumentBuffer offset:0];
        
        for (int32 i = 0 ; i < (int32)Arguments.size() ; ++ i)
        {
            FRenderResourcePtr Arg = Arguments[i];
            if (Arg->GetResourceType() == RRT_UNIFORM_BUFFER)
            {
                FUniformBufferPtr ArgUB = static_cast<FUniformBuffer*>(Arg.get());
                FUniformBufferMetal* ArgUBMetal = static_cast<FUniformBufferMetal*>(ArgUB.get());
                [ArgumentEncoder setBuffer:ArgUBMetal->Buffer offset:0 atIndex:i];
            }
            else if (Arg->GetResourceType() == RRT_TEXTURE)
            {
                FTexturePtr ArgTex = static_cast<FTexture*>(Arg.get());
                FTextureMetal* ArgTexMetal = static_cast<FTextureMetal*>(ArgTex.get());
                [ArgumentEncoder setTexture:ArgTexMetal->Texture atIndex:i];
            }
            else
            {
                _LOG(Fatal, "Invalid resource type in Argument buffer.\n");
            }
        }

        return true;
    }
    
    void FRHIMetal::PrepareDataForCPU(FUniformBufferPtr UniformBuffer)
    {
        // Metal iOS do not need sync data to CPU manually.
    }
    
    bool FRHIMetal::CopyTextureRegion(FTexturePtr DstTexture, const recti& InDstRegion, FTexturePtr SrcTexture)
    {
        FTextureMetal * DstMetalTexture = static_cast<FTextureMetal*>(DstTexture.get());
        FTextureMetal * SrcMetalTexture = static_cast<FTextureMetal*>(SrcTexture.get());
        TI_ASSERT(InDstRegion.getWidth() == SrcTexture->GetWidth() && InDstRegion.getHeight() == SrcTexture->GetHeight());
        MTLRegion SrcRegion = MTLRegionMake2D(0, 0, InDstRegion.getWidth(), InDstRegion.getHeight());
        MTLRegion DstRegion = MTLRegionMake2D(InDstRegion.Left, InDstRegion.Upper, InDstRegion.getWidth(), InDstRegion.getHeight());
        
        [BlitEncoder copyFromTexture: SrcMetalTexture->Texture
                         sourceSlice: 0
                         sourceLevel: 0
                        sourceOrigin: SrcRegion.origin
                          sourceSize: SrcRegion.size
                           toTexture: DstMetalTexture->Texture
                    destinationSlice: 0
                    destinationLevel: 0
                   destinationOrigin: DstRegion.origin];
        
        return true;
    }
    
    bool FRHIMetal::CopyBufferRegion(FUniformBufferPtr DstBuffer, uint32 DstOffset, FUniformBufferPtr SrcBuffer, uint32 Length)
    {
        FUniformBufferMetal * DstMetalBuffer = static_cast<FUniformBufferMetal*>(DstBuffer.get());
        FUniformBufferMetal * SrcMetalBuffer = static_cast<FUniformBufferMetal*>(SrcBuffer.get());
        
        [BlitEncoder copyFromBuffer: SrcMetalBuffer->Buffer
                       sourceOffset: 0
                           toBuffer: DstMetalBuffer->Buffer
                  destinationOffset: DstOffset
                               size: Length];
        return true;
    }
    
	void FRHIMetal::PutConstantBufferInHeap(FUniformBufferPtr InUniformBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot)
	{
        TI_ASSERT(0);
	}

	void FRHIMetal::PutTextureInHeap(FTexturePtr InTexture, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot)
	{
        TI_ASSERT(0);
	}
    
    void FRHIMetal::PutBufferInHeap(FUniformBufferPtr InBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot)
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

	void FRHIMetal::SetGraphicsPipeline(FPipelinePtr InPipeline)
	{
        // Metal need to set pipeline every time.
        //if (CurrentBoundResource.Pipeline != InPipeline)
        {
            TI_ASSERT(RenderEncoder != nil);
            FPipelineMetal* PLMetal = static_cast<FPipelineMetal*>(InPipeline.get());
            
            [RenderEncoder setRenderPipelineState:PLMetal->RenderPipelineState];
            [RenderEncoder setDepthStencilState:PLMetal->DepthState];
            
            HoldResourceReference(InPipeline);
            //CurrentBoundResource.Pipeline = InPipeline;
        }
        
        //E_CULL_MODE Cull = (E_CULL_MODE)InPipeline->GetDesc().RasterizerDesc.CullMode;
        //[RenderEncoder setFrontFacingWinding:MTLWindingClockwise];
        //[RenderEncoder setCullMode:k_CULL_MODE_MAP[Cull]];
	}

	void FRHIMetal::SetMeshBuffer(FMeshBufferPtr InMeshBuffer, FInstanceBufferPtr InInstanceBuffer)
    {
        FMeshBufferMetal* MBMetal = static_cast<FMeshBufferMetal*>(InMeshBuffer.get());
        TI_ASSERT(RenderEncoder != nil);
        [RenderEncoder setVertexBuffer:MBMetal->VertexBuffer offset:0 atIndex:0];
        if (InInstanceBuffer != nullptr)
        {
            FInstanceBufferMetal* IBMetal = static_cast<FInstanceBufferMetal*>(InInstanceBuffer.get());
            [RenderEncoder setVertexBuffer:IBMetal->InstanceBuffer offset:0 atIndex:1];
        }
	}

	void FRHIMetal::SetUniformBuffer(E_SHADER_STAGE ShaderStage, int32 BindIndex, FUniformBufferPtr InUniformBuffer)
	{
        FUniformBufferMetal * UBMetal = static_cast<FUniformBufferMetal*>(InUniformBuffer.get());
        if (ShaderStage == ESS_VERTEX_SHADER)
        {
            [RenderEncoder setVertexBuffer:UBMetal->Buffer offset:0 atIndex:BindIndex];
        }
        else if (ShaderStage == ESS_PIXEL_SHADER)
        {
            [RenderEncoder setFragmentBuffer:UBMetal->Buffer offset:0 atIndex:BindIndex];
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
    
    void FRHIMetal::SetArgumentBuffer(int32 BindIndex, FArgumentBufferPtr InArgumentBuffer)
    {
        FArgumentBufferMetal * ABMetal = static_cast<FArgumentBufferMetal*>(InArgumentBuffer.get());
        TI_ASSERT(BindIndex >= 0 && BindIndex < 31);
        
        // Indicate buffers and textures usage
        const TVector<FRenderResourcePtr>& Arguments = InArgumentBuffer->GetArguments();
        for (auto& Arg : Arguments)
        {
            if (Arg->GetResourceType() == RRT_UNIFORM_BUFFER)
            {
                FUniformBufferPtr UB = static_cast<FUniformBuffer*>(Arg.get());
                FUniformBufferMetal * UBMetal = static_cast<FUniformBufferMetal*>(UB.get());
                [RenderEncoder useResource: UBMetal->Buffer usage: MTLResourceUsageRead];
            }
            else if (Arg->GetResourceType() == RRT_TEXTURE)
            {
                FTexturePtr Texture = static_cast<FTexture*>(Arg.get());
                FTextureMetal * TexMetal = static_cast<FTextureMetal*>(Texture.get());
                [RenderEncoder useResource: TexMetal->Texture usage: MTLResourceUsageRead];
            }
            else
            {
                // Invalid resource type here.
                TI_ASSERT(0);
            }
        }

        // Set argument buffer
        [RenderEncoder setFragmentBuffer: ABMetal->ArgumentBuffer
                                  offset: 0
                                 atIndex: BindIndex];
    }
    
    void FRHIMetal::SetArgumentBuffer(FShaderBindingPtr InShaderBinding, FArgumentBufferPtr InArgumentBuffer)
    {
        FArgumentBufferMetal * ABMetal = static_cast<FArgumentBufferMetal*>(InArgumentBuffer.get());
        int32 BindIndex = InShaderBinding->GetPixelArgumentBufferBindingIndex();
        if (BindIndex >= 0)
        {
            // Indicate buffers and textures usage
            const TVector<FRenderResourcePtr>& Arguments = InArgumentBuffer->GetArguments();
            for (auto& Arg : Arguments)
            {
                if (Arg->GetResourceType() == RRT_UNIFORM_BUFFER)
                {
                    FUniformBufferPtr UB = static_cast<FUniformBuffer*>(Arg.get());
                    FUniformBufferMetal * UBMetal = static_cast<FUniformBufferMetal*>(UB.get());
                    [RenderEncoder useResource: UBMetal->Buffer usage: MTLResourceUsageRead];
                }
                else if (Arg->GetResourceType() == RRT_TEXTURE)
                {
                    FTexturePtr Texture = static_cast<FTexture*>(Arg.get());
                    FTextureMetal * TexMetal = static_cast<FTextureMetal*>(Texture.get());
                    [RenderEncoder useResource: TexMetal->Texture usage: MTLResourceUsageRead];
                }
                else
                {
                    // Invalid resource type here.
                    TI_ASSERT(0);
                }
            }
            // Set argument buffer
            [RenderEncoder setFragmentBuffer: ABMetal->ArgumentBuffer
                                      offset: 0
                                     atIndex: BindIndex];
        }
    }
    
    void FRHIMetal::SetResourceStateUB(FUniformBufferPtr InUniformBuffer, E_RESOURCE_STATE NewState)
    {
        // Metal do not have resource barrier.
    }

	void FRHIMetal::SetStencilRef(uint32 InRefValue)
	{
        if (RenderEncoder != nil)
        {
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
    
    void FRHIMetal::GraphicsCopyBuffer(FUniformBufferPtr Dest, uint32 DestOffset, FUniformBufferPtr Src, uint32 SrcOffset, uint32 CopySize)
    {
        TI_ASSERT(0);
    }
    
    void FRHIMetal::SetComputePipeline(FPipelinePtr InPipeline)
    {
        TI_TODO("Don't set pipeline duplicated.");
        TI_ASSERT(ComputeEncoder != nil);
        FPipelineMetal* PLMetal = static_cast<FPipelineMetal*>(InPipeline.get());
        [ComputeEncoder setComputePipelineState:PLMetal->ComputePipelineState];
    }
    
    void FRHIMetal::SetComputeConstantBuffer(int32 BindIndex, FUniformBufferPtr InUniformBuffer)
    {
        TI_ASSERT(0);
    }
    
    void FRHIMetal::SetComputeResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable)
    {
        TI_ASSERT(0);
    }
    
    void FRHIMetal::SetComputeArgumentBuffer(int32 BindIndex, FArgumentBufferPtr InArgumentBuffer)
    {
        TI_ASSERT(ComputeEncoder != nil);
        FArgumentBufferMetal * ABMetal = static_cast<FArgumentBufferMetal*>(InArgumentBuffer.get());
        TI_ASSERT(BindIndex >= 0 && BindIndex < 31);

        // Indicate buffers and textures usage
        const TVector<FRenderResourcePtr>& Arguments = InArgumentBuffer->GetArguments();
        for (auto& Arg : Arguments)
        {
            if (Arg->GetResourceType() == RRT_UNIFORM_BUFFER)
            {
                FUniformBufferPtr UB = static_cast<FUniformBuffer*>(Arg.get());
                FUniformBufferMetal * UBMetal = static_cast<FUniformBufferMetal*>(UB.get());
                [ComputeEncoder useResource: UBMetal->Buffer usage: MTLResourceUsageRead];
            }
            else if (Arg->GetResourceType() == RRT_TEXTURE)
            {
                FTexturePtr Texture = static_cast<FTexture*>(Arg.get());
                FTextureMetal * TexMetal = static_cast<FTextureMetal*>(Texture.get());
                [ComputeEncoder useResource: TexMetal->Texture usage: MTLResourceUsageRead];
            }
            else
            {
                // Invalid resource type here.
                TI_ASSERT(0);
            }
        }
        
        // Set argument buffer
        [ComputeEncoder setBuffer: ABMetal->ArgumentBuffer
                           offset: 0
                          atIndex: BindIndex];
    }
    
    void FRHIMetal::SetComputeArgumentBuffer(FShaderBindingPtr InShaderBinding, FArgumentBufferPtr InArgumentBuffer)
    {
        TI_ASSERT(0);
    }
    
    void FRHIMetal::DispatchCompute(const vector3di& GroupSize, const vector3di& GroupCount)
    {
        TI_ASSERT(ComputeEncoder != nil);
        MTLSize ThreadGroupSize, ThreadGroupCount;
        ThreadGroupSize.width = GroupSize.X;
        ThreadGroupSize.height = GroupSize.Y;
        ThreadGroupSize.depth = GroupSize.Z;
        ThreadGroupCount.width = GroupCount.X;
        ThreadGroupCount.height = GroupCount.Y;
        ThreadGroupCount.depth = GroupCount.Z;
        
        [ComputeEncoder dispatchThreadgroups:ThreadGroupCount
                       threadsPerThreadgroup:ThreadGroupSize];
    }
    
    void FRHIMetal::ComputeCopyBuffer(FUniformBufferPtr Dest, uint32 DestOffset, FUniformBufferPtr Src, uint32 SrcOffset, uint32 CopySize)
    {
        TI_ASSERT(0);
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

	void FRHIMetal::BeginRenderToRenderTarget(FRenderTargetPtr RT, const int8* PassName)
    {
        CloseCurrentEncoderIfNotMatch(EPL_GRAPHICS);
        
        if (CurrentRenderTarget != RT)
        {
            if (RenderEncoder != nil)
            {
                [RenderEncoder endEncoding];
            }
            FRHI::BeginRenderToRenderTarget(RT, PassName);
            
            FRenderTargetMetal * RTMetal = static_cast<FRenderTargetMetal*>(RT.get());
            RenderEncoder = [CommandBuffer renderCommandEncoderWithDescriptor:RTMetal->RenderPassDesc];
#if defined (TIX_DEBUG)
            RenderEncoder.label = [NSString stringWithUTF8String:PassName];
#endif
            CurrentCommandListState.ListType = EPL_GRAPHICS;
            
            // Set scissor rect
            const FViewport& VP = RtViewport;
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
	}
    
    void FRHIMetal::HoldResourceReference(FRenderResourcePtr InResource)
    {
        ResHolders[CurrentFrame]->HoldReference(InResource);
    }
    
    void FRHIMetal::HoldResourceReference(id<MTLBuffer> InBuffer)
    {
        ResHolders[CurrentFrame]->HoldMetalBufferReference(InBuffer);
    }
}
#endif	// COMPILE_WITH_RHI_METAL
