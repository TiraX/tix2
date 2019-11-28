/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FSceneTileResource : public FRenderResource
	{
	public:
		FSceneTileResource();
		FSceneTileResource(const TSceneTileResource& InSceneTileResource);
		virtual ~FSceneTileResource();

		const vector2di& GetTilePosition() const
		{
			return Position;
		}

		const aabbox3df& GetTileBBox() const
		{
			return BBox;
		}

		const TVector<vector2di>& GetInstanceCountAndOffset() const
		{
			return InstanceCountAndOffset;
		}

		FInstanceBufferPtr GetInstanceBuffer()
		{
			return InstanceBuffer;
		}

		uint32 GetTotalMeshes() const
		{
			return TotalMeshes;
		}

		uint32 GetTotalMeshSections() const
		{
			return TotalMeshSections;
		}

		uint32 GetTotalInstances() const
		{
			return TotalInstances;
		}

		void AddPrimitive(uint32 Index, FPrimitivePtr Primitive);

		const TVector<FPrimitivePtr>& GetPrimitives() const
		{
			return Primitives;
		}

	private:
		vector2di Position;
		aabbox3df BBox;
		uint32 TotalMeshes;
		uint32 TotalMeshSections;
		uint32 TotalInstances;
		// X is Count, Y is Offset
		TVector<vector2di> InstanceCountAndOffset;
		FInstanceBufferPtr InstanceBuffer;

		TVector<FPrimitivePtr> Primitives;
	};
}
