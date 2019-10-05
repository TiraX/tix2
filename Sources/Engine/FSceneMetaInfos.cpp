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
		, ActiveScenePrimitiveInfos(0)
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

		ScenePrimitiveMetaInfo = ti_new FScenePrimitiveMetaInfo;
		ScenePrimitiveMetaInfo->InitToZero();
		ActiveScenePrimitiveInfos = 0;
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

		// Mark tile as dirty
		SceneMetaFlags |= MetaFlag_SceneTileMetaDirty;
	}

	void FSceneMetaInfos::RegisterPrimitive(FPrimitivePtr InPrimitive)
	{
		FUInt4 PrimitiveInfo;

		const vector2di& ParentTilePosition = InPrimitive->GetParentTilePosition();
		TI_ASSERT(SceneTileMetaIndexMap.find(ParentTilePosition) != SceneTileMetaIndexMap.end());
		PrimitiveInfo.X = (int32)SceneTileMetaIndexMap[ParentTilePosition];	// Scene tile meta info index

		ScenePrimitiveMetaInfo->UniformBufferData[ActiveScenePrimitiveInfos].Info = PrimitiveInfo;
		++ActiveScenePrimitiveInfos;
		TI_ASSERT(ActiveScenePrimitiveInfos < MAX_DRAW_CALL_IN_SCENE);

		// Mark primitives as dirty
		SceneMetaFlags |= MetaFlag_ScenePrimitiveMetaDirty;
	}

	void FSceneMetaInfos::UpdateGPUResources()
	{
		if (HasMetaFlag(MetaFlag_SceneTileMetaDirty))
		{
			SceneTileMetaInfo->InitUniformBuffer();
			// MetaFlag Clear at FScene::ClearSceneFlags();
		}
		if (HasMetaFlag(MetaFlag_ScenePrimitiveMetaDirty))
		{
			ScenePrimitiveMetaInfo->InitUniformBuffer();
		}
	}

	void FSceneMetaInfos::ClearMetaFlags()
	{
		SceneMetaFlags = 0;
	}
}
