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

	void FPrimitive::AddMesh(FMeshBufferPtr MeshBuffer, FPipelinePtr Pipeline, TMaterialInstancePtr MInstance)
	{
		MeshBuffers.push_back(MeshBuffer);
		Pipelines.push_back(Pipeline);

		Uniforms.push_back(MInstance->UniformBuffer);

		const TVector<TTexturePtr>& TextureParams = MInstance->GetTextureParams();
		for (const auto& t : TextureParams)
		{
			Textures.push_back(t->TextureResource);
		}

		TI_ASSERT(MeshBuffers.size() == Uniforms.size() && MeshBuffers.size() == Pipelines.size());
	}
}
