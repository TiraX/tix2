/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FPrimitive : public IReferenceCounted
	{
	public:
		FPrimitive();
		virtual ~FPrimitive();

		void AddMesh(FMeshBufferPtr MeshBuffer, FPipelinePtr Pipeline, FUniformBufferPtr UniformBuffer);

		TVector<FMeshBufferPtr> MeshBuffers;
		TVector<FPipelinePtr> Pipelines;
		TVector<FUniformBufferPtr> Uniforms;
	};
} // end namespace tix

