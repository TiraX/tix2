/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FPrimitive.h"

namespace tix
{
	FPrimitive::FPrimitive()
	{
	}

	FPrimitive::~FPrimitive()
	{
	}

	void FPrimitive::AddMesh(FMeshBufferPtr InMeshBuffer, const aabbox3df& InMeshBBox, TMaterialInstancePtr InMInstance)
	{
		// Add bbox
		if (MeshBuffers.empty())
		{
			BBox.reset(InMeshBBox);
		}
		else
		{
			BBox.addInternalBox(InMeshBBox);
		}

		// Add mesh buffer
		MeshBuffers.push_back(InMeshBuffer);

		// Add pipeline
		TMaterialPtr Material = InMInstance->LinkedMaterial;
		TI_ASSERT(Material->PipelineResource != nullptr);
		Pipelines.push_back(Material->PipelineResource);

		// Instance uniform buffer
		Uniforms.push_back(InMInstance->UniformBuffer);

		// Texture resource table
		TextureTables.push_back(InMInstance->TextureResourceTable);

		TI_ASSERT(MeshBuffers.size() == Uniforms.size() && 
			MeshBuffers.size() == Pipelines.size() &&
			MeshBuffers.size() == TextureTables.size());
	}
}
