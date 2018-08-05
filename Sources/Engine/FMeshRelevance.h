/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct FMeshRelevance
	{
		// Mesh buffer
		FMeshBufferPtr MeshBuffer;

		// Pipeline
		FPipelinePtr Pipeline;

		// Uniform buffer
		FUniformBufferPtr UniformBuffer;
	};
}
