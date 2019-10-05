/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	BEGIN_UNIFORM_BUFFER_STRUCT(FPrimitiveUniformBuffer)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FMatrix, LocalToWorld)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, VTUVTransform)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, VTDebugInfo)
	END_UNIFORM_BUFFER_STRUCT(FPrimitiveUniformBuffer)

	class FPrimitive : public IReferenceCounted
	{
	public:
		enum {
			PrimitiveUniformBufferDirty = 1 << 0,
		};

		FPrimitive();
		~FPrimitive();

		void SetMesh(FMeshBufferPtr InMeshBuffer, const aabbox3df& InMeshBBox, TMaterialInstancePtr InMInstance, FInstanceBufferPtr InInstanceBuffer);
		void SetParentTilePosition(const vector2di& InTilePos)
		{
			ParentTilePos = InTilePos;
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
		bool IsPrimitiveBufferDirty() const
		{
			return (PrimitiveFlag & PrimitiveUniformBufferDirty) != 0;
		}
		const vector2di& GetParentTilePosition() const
		{
			return ParentTilePos;
		}
		void SetLocalToWorld(const matrix4 InLocalToWorld);
		void SetUVTransform(float UOffset, float VOffset, float UScale, float VScale);
		void SetVTDebugInfo(float A, float B, float C, float D);

		void UpdatePrimitiveBuffer_RenderThread();
	private:
		uint32 PrimitiveFlag;

		FMeshBufferPtr MeshBuffer;
		FInstanceBufferPtr InstanceBuffer;

		FPipelinePtr Pipeline;
		FArgumentBufferPtr Argument;

		// Every primitive has a unique Primitive uniform buffer, because VT UVTransform
		FPrimitiveUniformBufferPtr PrimitiveUniformBuffer;
		aabbox3df BBox;

		E_DRAWLIST_TYPE DrawList;
		vector2di ParentTilePos;
	};
} // end namespace tix

