/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	BEGIN_UNIFORM_BUFFER_STRUCT(FPrimitiveUniformBuffer)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FInt4, LightsNum)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FInt4, LightIndices)
	END_UNIFORM_BUFFER_STRUCT(FPrimitiveUniformBuffer)

	class FPrimitive : public IReferenceCounted
	{
	public:
		FPrimitive();
		~FPrimitive();

		void AddMesh(FMeshBufferPtr InMeshBuffer, const aabbox3df& InMeshBBox, TMaterialInstancePtr InMInstance, FInstanceBufferPtr InInstanceBuffer);

		void SetPrimitiveUniform(FPrimitiveUniformBufferPtr InUniform)
		{
			PrimitiveUniformBuffer = InUniform;
		}

		FMeshBufferPtr GetMeshBuffer()
		{
			return MeshBuffer;
		}
		FInstanceBufferPtr GetInstanceBuffer()
		{
			return InstanceBuffer;
		}
		FPipelinePtr GetPipeline()
		{
			return Pipeline;
		}
		FArgumentBufferPtr GetArgumentBuffer()
		{
			return Argument;
		}
		FPrimitiveUniformBufferPtr GetPrimitiveUniform()
		{
			return PrimitiveUniformBuffer;
		}
		const aabbox3df& GetBBox() const
		{
			return BBox;
		}
		E_DRAWLIST_TYPE GetDrawList() const
		{
			return DrawList;
		}

	private:
		FMeshBufferPtr MeshBuffer;
		FInstanceBufferPtr InstanceBuffer;

		FPipelinePtr Pipeline;
		FArgumentBufferPtr Argument;

		FPrimitiveUniformBufferPtr PrimitiveUniformBuffer;
		aabbox3df BBox;

		E_DRAWLIST_TYPE DrawList;
	};
} // end namespace tix

