/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FPrimitive.h"

namespace tix
{
	FPrimitive::FPrimitive()
		: PrimitiveFlag(0)
		, IndexStart(0)
		, Triangles(0)
		, InstanceCount(0)
		, InstanceOffset(0)
		, GlobalInstanceOffset(0)
		, DrawList(LIST_INVALID)
	{
		PrimitiveUniformBuffer = ti_new FPrimitiveUniformBuffer;
	}

	FPrimitive::~FPrimitive()
	{
		TI_ASSERT(IsRenderThread());
		PrimitiveUniformBuffer = nullptr;
	}

	void FPrimitive::SetMesh(
		FMeshBufferPtr InMeshBuffer, 
		uint32 InIndexStart,
		uint32 InTriangles,
		TMaterialInstancePtr InMInstance, 
		FInstanceBufferPtr InInstanceBuffer,
		uint32 InInstanceCount,
		uint32 InInstanceOffset)
	{
		// Add mesh buffer
		MeshBuffer = InMeshBuffer;
		IndexStart = InIndexStart;
		Triangles = InTriangles;

		// Add instance buffer
		InstanceBuffer = InInstanceBuffer;
		InstanceCount = InInstanceCount;
		InstanceOffset = InInstanceOffset;

		// Add pipeline
		TMaterialPtr Material = InMInstance->LinkedMaterial;
		TI_ASSERT(Material->PipelineResource != nullptr);
		Pipeline = Material->PipelineResource;

		// Instance material argument buffer
		Argument = InMInstance->ArgumentBuffer;

		// Draw List
		if (Material->GetBlendMode() == BLEND_MODE_OPAQUE)
		{
			DrawList = LIST_OPAQUE;
		}
		else if (Material->GetBlendMode() == BLEND_MODE_MASK)
		{
			DrawList = LIST_MASK;
		}
		else
		{
			DrawList = LIST_TRANSLUCENT;
		}
	}

	void FPrimitive::SetLocalToWorld(const matrix4 InLocalToWorld)
	{
		TI_ASSERT(IsRenderThread());
		PrimitiveUniformBuffer->UniformBufferData[0].LocalToWorld = InLocalToWorld;
		PrimitiveFlag |= PrimitiveUniformBufferDirty;
	}

	void FPrimitive::SetUVTransform(float UOffset, float VOffset, float UScale, float VScale)
	{
		TI_ASSERT(IsRenderThread());
		PrimitiveUniformBuffer->UniformBufferData[0].VTUVTransform = FFloat4(UOffset, VOffset, UScale, VScale);
		PrimitiveFlag |= PrimitiveUniformBufferDirty;
	}

	void FPrimitive::SetVTDebugInfo(float A, float B, float C, float D)
	{
		TI_ASSERT(IsRenderThread());
		PrimitiveUniformBuffer->UniformBufferData[0].VTDebugInfo = FFloat4(A, B, C, D);
		PrimitiveFlag |= PrimitiveUniformBufferDirty;
	}

	void FPrimitive::UpdatePrimitiveBuffer_RenderThread()
	{
		TI_ASSERT(IsRenderThread());
		PrimitiveUniformBuffer->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
		PrimitiveFlag &= ~PrimitiveUniformBufferDirty;
	}
}
