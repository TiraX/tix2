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
		void AddMesh(FMeshBufferPtr InMeshBuffer, const aabbox3df& InMeshBBox, TMaterialInstancePtr InMInstance);

		TVector<FMeshBufferPtr> MeshBuffers;
		TVector<FPipelinePtr> Pipelines;
		TVector<FUniformBufferPtr> Uniforms;
		aabbox3df BBox;
		// Temp solution, re-factor in future
		TVector<FTexturePtr> Textures;


	};
} // end namespace tix

