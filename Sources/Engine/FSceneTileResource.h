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
		FSceneTileResource(TSceneTileResourcePtr InSceneTileResource);
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

	private:
		vector2di Position;
		aabbox3df BBox;
		// X is Count, Y is Offset
		TVector<vector2di> InstanceCountAndOffset;
		FInstanceBufferPtr InstanceBuffer;
	};
}
