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
        TI_ASSERT(0);
	}

	void FRHIMetal::EndFrame()
	{
        TI_ASSERT(0);
	}

	FTexturePtr FRHIMetal::CreateTexture()
	{
        TI_ASSERT(0);
        return nullptr;
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
        TI_ASSERT(0);
        return nullptr;
	}

	FPipelinePtr FRHIMetal::CreatePipeline()
	{
        TI_ASSERT(0);
        return nullptr;
	}

	//FRenderTargetPtr FRHIMetal::CreateRenderTarget(int32 W, int32 H)
	//{
	//	return ti_new FRenderTargetDx12(W, H);
	//}

	FShaderBindingPtr FRHIMetal::CreateShaderBinding(uint32 NumBindings)
	{
        TI_ASSERT(0);
        return nullptr;
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
        TI_ASSERT(0);

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
        TI_ASSERT(0);
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
