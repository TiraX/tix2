/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	BEGIN_UNIFORM_BUFFER_STRUCT(FLightBindingUniformBuffer)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FInt4, LightsNum)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FInt4, LightIndices)
	END_UNIFORM_BUFFER_STRUCT(FLightBindingUniformBuffer)

	class FPrimitive : public IReferenceCounted
	{
	public:
		FPrimitive();
		~FPrimitive();

		// Temp solution, re-factor in future
		void AddMesh(FMeshBufferPtr InMeshBuffer, const aabbox3df& InMeshBBox, TMaterialInstancePtr InMInstance);

		TVector<FMeshBufferPtr> MeshBuffers;
		TVector<FPipelinePtr> Pipelines;
		TVector<FArgumentBufferPtr> Arguments;

		FLightBindingUniformBufferPtr LightBindingUniformBuffer;
		aabbox3df BBox;
	};
} // end namespace tix

