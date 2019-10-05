/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FSceneMetaInfos.h"

namespace tix
{
	FSceneMetaInfos::FSceneMetaInfos()
		: SceneMetaFlags(0)
		, ActiveSceneTileInfos(0)
	{
		Init();
	}

	FSceneMetaInfos::~FSceneMetaInfos()
	{
		SceneTileMetaInfo = nullptr;
	}

	void FSceneMetaInfos::Init()
	{
		// Create a mesh buffer
		FRHI * RHI = FRHI::Get();

		SceneTileMetaInfo = ti_new FSceneTileMetaInfo;
		SceneTileMetaInfo->InitToZero();
		ActiveSceneTileInfos = 0;
	}

	void FSceneMetaInfos::RegisterSceneTile(const vector2di& TilePos, const aabbox3df& TileBBox)
	{
		TI_ASSERT(SceneTileMetaIndexMap.find(TilePos) == SceneTileMetaIndexMap.end());
		// Set tile meta info data
		SceneTileMetaInfo->UniformBufferData[ActiveSceneTileInfos].MinEdge = FFloat4(TileBBox.MinEdge.X, TileBBox.MinEdge.Y, TileBBox.MinEdge.Z, 1.f);
		SceneTileMetaInfo->UniformBufferData[ActiveSceneTileInfos].MaxEdge = FFloat4(TileBBox.MaxEdge.X, TileBBox.MaxEdge.Y, TileBBox.MaxEdge.Z, 1.f);

		// Remember the meta index of this tile
		SceneTileMetaIndexMap[TilePos] = ActiveSceneTileInfos;
		++ActiveSceneTileInfos;
		TI_ASSERT(ActiveSceneTileInfos < MAX_SCENE_TILE_META_NUM);

		// Mark as dirty
		SceneMetaFlags |= MetaFlag_SceneTileMetaDirty;
	}

	void FSceneMetaInfos::UpdateGPUResources()
	{
		if (HasMetaFlag(MetaFlag_SceneTileMetaDirty))
		{
			SceneTileMetaInfo->InitUniformBuffer();
			// SceneMetaFlags &= ~MetaFlag_SceneTileMetaDirty;
		}
	}

	void FSceneMetaInfos::ClearMetaFlags()
	{
		SceneMetaFlags = 0;
	}
}
