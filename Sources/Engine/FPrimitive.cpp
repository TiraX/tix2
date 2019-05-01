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
		, DrawList(LIST_INVALID)
	{
		PrimitiveUniformBuffer = ti_new FPrimitiveUniformBuffer;
	}

	FPrimitive::~FPrimitive()
	{
		TI_ASSERT(IsRenderThread());
		PrimitiveUniformBuffer = nullptr;
	}

	void FPrimitive::SetMesh(FMeshBufferPtr InMeshBuffer, const aabbox3df& InMeshBBox, TMaterialInstancePtr InMInstance, FInstanceBufferPtr InInstanceBuffer)
	{
		// Add bounding box
		BBox = InMeshBBox;

		// Add mesh buffer
		MeshBuffer = InMeshBuffer;

		// Add instance buffer
		InstanceBuffer = InInstanceBuffer;

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

	void FPrimitive::SetWorldTransform(const matrix4 InWorldTransform)
	{
		TI_ASSERT(IsRenderThread());
		PrimitiveUniformBuffer->UniformBufferData[0].WorldTransform = InWorldTransform;
		PrimitiveFlag |= PrimitiveUniformBufferDirty;
	}

	void FPrimitive::SetUVTransform(float UOffset, float VOffset, float UScale, float VScale)
	{
		TI_ASSERT(IsRenderThread());
		PrimitiveUniformBuffer->UniformBufferData[0].VTUVTransform = FFloat4(UOffset, VOffset, UScale, VScale);
		PrimitiveFlag |= PrimitiveUniformBufferDirty;
	}

	void FPrimitive::UpdatePrimitiveBuffer_RenderThread()
	{
		TI_ASSERT(IsRenderThread());
		PrimitiveUniformBuffer->InitUniformBuffer();
		PrimitiveFlag &= ~PrimitiveUniformBufferDirty;
	}
}
