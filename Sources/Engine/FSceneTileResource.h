/*
	TiX Engine v2.0 Copyright (C) 2018~2021
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

		uint32 GetTotalStaticMeshes() const
		{
			return TotalStaticMeshes;
		}

		uint32 GetTotalMeshSections() const
		{
			return TotalSMSections;
		}

		uint32 GetTotalInstances() const
		{
			return TotalSMInstances;
		}

		void AddPrimitive(uint32 Index, FPrimitivePtr Primitive);
		void AppendPrimitive(FPrimitivePtr Primitive);

		const TVector<FPrimitivePtr>& GetPrimitives() const
		{
			return Primitives;
		}

	private:
		vector2di Position;
		aabbox3df BBox;
		uint32 TotalStaticMeshes;
		uint32 TotalSMSections;
		uint32 TotalSMInstances;
		// X is Count, Y is Offset
		TVector<vector2di> InstanceCountAndOffset;
		FInstanceBufferPtr InstanceBuffer;

		TVector<FPrimitivePtr> Primitives;
	};
}
