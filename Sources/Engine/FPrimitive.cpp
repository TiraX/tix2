/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FPrimitive.h"

namespace tix
{
	FPrimitive::FPrimitive()
		: DrawList(LIST_INVALID)
	{
	}

	FPrimitive::~FPrimitive()
	{
	}

	void FPrimitive::AddMesh(FMeshBufferPtr InMeshBuffer, const aabbox3df& InMeshBBox, TMaterialInstancePtr InMInstance, FInstanceBufferPtr InInstanceBuffer)
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
}
