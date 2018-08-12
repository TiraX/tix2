/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include "FRHIConfig.h"

namespace tix
{
	enum E_RHI_TYPE
	{
		ERHI_DX12 = 0,

		ERHI_NUM,
	};

	class FFrameResources;
	// Render hardware interface
	class FRHI
	{
	public: 
		static FRHI* Get();
		static void CreateRHI(E_RHI_TYPE RhiType);
		static void ReleaseRHI();

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void WaitingForGpu() = 0;

		virtual FTexturePtr CreateTexture() = 0;
		virtual FTexturePtr CreateTexture(E_PIXEL_FORMAT Format, int32 Width, int32 Height) = 0;
		virtual FMeshBufferPtr CreateMeshBuffer() = 0;
		virtual FPipelinePtr CreatePipeline() = 0;
		virtual FUniformBufferPtr CreateUniformBuffer() = 0;

		virtual bool UpdateHardwareBuffer(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData) = 0;
		virtual bool UpdateHardwareBuffer(FTexturePtr Texture, TTexturePtr InTexData) = 0;
		virtual bool UpdateHardwareBuffer(FPipelinePtr Pipeline, TPipelinePtr InPipelineDesc) = 0;
		virtual bool UpdateHardwareBuffer(FUniformBufferPtr UniformBuffer, void* InData, int32 InDataSize) = 0;

		virtual void SetMeshBuffer(FMeshBufferPtr InMeshBuffer) = 0;
		virtual void SetPipeline(FPipelinePtr InPipeline) = 0;
		virtual void SetUniformBuffer(FUniformBufferPtr InUniformBuffer) = 0;

		virtual void DrawPrimitiveIndexedInstanced(
			uint32 IndexCountPerInstance,
			uint32 InstanceCount,
			uint32 StartIndexLocation,
			int32 BaseVertexLocation,
			uint32 StartInstanceLocation) = 0;

		E_RHI_TYPE GetRHIType() const
		{
			return RHIType;
		}

	protected:
		static FRHI* RHI;
		FRHI(E_RHI_TYPE InRHIType);
		virtual ~FRHI();

	protected:
		E_RHI_TYPE RHIType;
		FFrameResources * FrameResources[FRHIConfig::FrameBufferNum];
	};
}
