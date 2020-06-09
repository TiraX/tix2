/*
	TiX Engine v2.0 Copyright (C) 2018~2021
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

		void SetMesh(
			FMeshBufferPtr InMeshBuffer,
			uint32 InIndexStart,
			uint32 InTriangles,
			TMaterialInstancePtr InMInstance,
			FInstanceBufferPtr InInstanceBuffer,
			uint32 InInstanceCount,
			uint32 InInstanceOffset
		);

		FMeshBufferPtr GetMeshBuffer()
		{
			return MeshBuffer;
		}
		FInstanceBufferPtr GetInstanceBuffer()
		{
			return InstanceBuffer;
		}
		const uint32 GetIndexStart() const
		{
			return IndexStart;
		}
		const uint32 GetTriangles() const
		{
			return Triangles;
		}
		uint32 GetInstanceCount() const
		{
			return InstanceCount;
		}
		uint32 GetInstanceOffset() const
		{
			return InstanceOffset;
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
		E_DRAWLIST_TYPE GetDrawList() const
		{
			return DrawList;
		}
		bool IsPrimitiveBufferDirty() const
		{
			return (PrimitiveFlag & PrimitiveUniformBufferDirty) != 0;
		}
		const vector2di& GetSceneTilePos() const
		{
			return SceneTilePos;
		}
		void SetLocalToWorld(const matrix4 InLocalToWorld);
		void SetUVTransform(float UOffset, float VOffset, float UScale, float VScale);
		void SetVTDebugInfo(float A, float B, float C, float D);
		void SetSceneTilePos(const vector2di& InPos)
		{
			SceneTilePos = InPos;
		}

		void UpdatePrimitiveBuffer_RenderThread();
	private:
		uint32 PrimitiveFlag;

		FMeshBufferPtr MeshBuffer;
		uint32 IndexStart;
		uint32 Triangles;
		FInstanceBufferPtr InstanceBuffer;
		uint32 InstanceCount;
		uint32 InstanceOffset;

		FPipelinePtr Pipeline;
		FArgumentBufferPtr Argument;

		// Every primitive has a unique Primitive uniform buffer, because VT UVTransform
		FPrimitiveUniformBufferPtr PrimitiveUniformBuffer;

		vector2di SceneTilePos;

		E_DRAWLIST_TYPE DrawList;
	};
} // end namespace tix

