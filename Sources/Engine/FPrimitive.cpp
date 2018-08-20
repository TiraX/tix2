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

	void FPrimitive::AddMesh(FMeshBufferPtr MeshBuffer, FPipelinePtr Pipeline, FUniformBufferPtr UniformBuffer)
	{
		MeshBuffers.push_back(MeshBuffer);
		Pipelines.push_back(Pipeline);
		Uniforms.push_back(UniformBuffer);

		TI_ASSERT(MeshBuffers.size() == Uniforms.size() && MeshBuffers.size() == Pipelines.size());
	}
}
