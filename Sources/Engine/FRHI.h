/*
	TiX Engine v2.0 Copyright (C) 2018~2019
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

		virtual void InitRHI() = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void WaitingForGpu() = 0;

		virtual FTexturePtr CreateTexture() = 0;
		virtual FTexturePtr CreateTexture(const TTextureDesc& Desc) = 0;
		virtual FUniformBufferPtr CreateUniformBuffer(uint32 InStructSize) = 0;
		virtual FMeshBufferPtr CreateMeshBuffer() = 0;
		virtual FPipelinePtr CreatePipeline() = 0;
		virtual FRenderTargetPtr CreateRenderTarget(int32 W, int32 H) = 0;
		virtual FRenderResourceTablePtr CreateRenderResourceTable(uint32 InSize);
		virtual FShaderBindingPtr CreateShaderBinding(uint32 NumBindings) = 0;
		virtual FShaderPtr CreateShader(const TShaderNames& InNames) = 0;
		virtual FArgumentBufferPtr CreateArgumentBuffer(FShaderPtr InShader) = 0;

		virtual bool UpdateHardwareResource(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData) = 0;
		virtual bool UpdateHardwareResource(FTexturePtr Texture) = 0;
		virtual bool UpdateHardwareResource(FTexturePtr Texture, TTexturePtr InTexData) = 0;
		virtual bool UpdateHardwareResource(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc) = 0;
		virtual bool UpdateHardwareResource(FUniformBufferPtr UniformBuffer, void* InData) = 0;
		virtual bool UpdateHardwareResource(FRenderTargetPtr RenderTarget) = 0;
		virtual bool UpdateHardwareResource(FShaderBindingPtr ShaderBindingResource, const TVector<TBindingParamInfo>& BindingInfos) = 0;
		virtual bool UpdateHardwareResource(FShaderPtr ShaderResource) = 0;
		virtual bool UpdateHardwareResource(FArgumentBufferPtr ArgumentBuffer, TStreamPtr ArgumentData, const TVector<FTexturePtr>& ArgumentTextures) = 0;

		virtual void PutUniformBufferInHeap(FUniformBufferPtr InUniformBuffer, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) = 0;
		virtual void PutTextureInHeap(FTexturePtr InTexture, E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 InHeapSlot) = 0;
		virtual void PutRTColorInHeap(FTexturePtr InTexture, uint32 InHeapSlot) = 0;
		virtual void PutRTDepthInHeap(FTexturePtr InTexture, uint32 InHeapSlot) = 0;

		virtual void SetPipeline(FPipelinePtr InPipeline) = 0;
		virtual void SetMeshBuffer(FMeshBufferPtr InMeshBuffer) = 0;
		virtual void SetUniformBuffer(int32 BindIndex, FUniformBufferPtr InUniformBuffer) = 0;
		virtual void SetRenderResourceTable(int32 BindIndex, FRenderResourceTablePtr RenderResourceTable) = 0;
		virtual void SetShaderTexture(int32 BindIndex, FTexturePtr InTexture) = 0;
		virtual void SetArgumentBuffer(FArgumentBufferPtr InArgumentBuffer) = 0;
		
		virtual void SetStencilRef(uint32 InRefValue) = 0;
		virtual void DrawPrimitiveIndexedInstanced(
			uint32 IndexCountPerInstance,
			uint32 InstanceCount,
			uint32 StartIndexLocation,
			int32 BaseVertexLocation,
			uint32 StartInstanceLocation) = 0;

		virtual void SetViewport(const FViewport& InViewport);
		virtual void PushRenderTarget(FRenderTargetPtr RT);
		virtual FRenderTargetPtr PopRenderTarget();

		E_RHI_TYPE GetRHIType() const
		{
			return RHIType;
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

	protected:
		E_RHI_TYPE RHIType;
		FViewport Viewport;
		FFrameResources * FrameResources[FRHIConfig::FrameBufferNum];

		TVector<FRenderTargetPtr> RenderTargets;
		TVector<FViewport> RtViewports;

		FRenderResourceHeap RenderResourceHeap[EHT_COUNT];
		FBoundResource CurrentBoundResource;
	};
}
