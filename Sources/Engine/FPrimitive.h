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

		// Temp solution, re-factor in future
		void AddMesh(FMeshBufferPtr MeshBuffer, FPipelinePtr Pipeline, TMaterialInstancePtr MInstance);

		TVector<FMeshBufferPtr> MeshBuffers;
		TVector<FPipelinePtr> Pipelines;
		TVector<FUniformBufferPtr> Uniforms;
		// Temp solution, re-factor in future
		TVector<FTexturePtr> Textures;

	};
} // end namespace tix

