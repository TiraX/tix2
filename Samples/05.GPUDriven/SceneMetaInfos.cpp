/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SceneMetaInfos.h"

namespace tix
{
#define LOG_LOADING_INFO (0)
#if LOG_LOADING_INFO
#	define SCENE_META_LOG _LOG
#else
#	define SCENE_META_LOG(...)
#endif

	FSceneMetaInfos::FSceneMetaInfos()
		: SceneMetaFlags(0)
		, SceneInstancesAdded(0)
		, SceneInstancesIntersected(0)
		, ScenePrimitivesAdded(0)
		, SceneTileIntersected(0)
		, SceneTileInner(0)
	{
		ActiveSceneTileInfoMap.reserve(128);
		SceneTileVisibleInfo.reserve(128);
		SortedTilePositions.reserve(128);
	}

	FSceneMetaInfos::~FSceneMetaInfos()
	{
	}

	void FSceneMetaInfos::DoSceneTileCulling(FScene * Scene, const SViewFrustum& ViewFrustum)
	{
		static bool CullEnabled = true;
		if (!CullEnabled)
		{
			return;
		}
		if (Scene->HasSceneFlag(FScene::SceneTileDirty) ||
			Scene->HasSceneFlag(FScene::ViewProjectionDirty))
		{
			SCENE_META_LOG(Log, "Cull tile. TileDirty %d; ViewDirty %d.\n", Scene->HasSceneFlag(FScene::SceneTileDirty) ? 1:0, Scene->HasSceneFlag(FScene::ViewProjectionDirty) ? 1:0);
			const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();
			if (Scene->HasSceneFlag(FScene::SceneTileDirty))
			{
				if (SceneTileVisibleInfo.size() != SceneTileResources.size())
				{
					SceneTileVisibleInfo.clear();
					SceneMetaFlags |= MetaFlag_SceneTileMetaDirty;
					for (const auto& T : SceneTileResources)
					{
						SceneTileVisibleInfo[T.first] = 0;
					}
				}
			}

			// Do CPU tile frustum culling
			for (const auto& T : SceneTileResources)
			{
				const vector2di& TilePos = T.first;
				FSceneTileResourcePtr TileRes = T.second;
				const aabbox3df& TileBBox = TileRes->GetTileBBox();
				
				E_CULLING_RESULT CullingResult = ViewFrustum.intersectsEx(TileBBox);
				if (SceneTileVisibleInfo[TilePos] != CullingResult)
				{
					SceneTileVisibleInfo[TilePos] = CullingResult;
					SceneMetaFlags |= MetaFlag_SceneTileMetaDirty;
				}
			}
		}
	}

	void FSceneMetaInfos::CollectSceneMetaInfos(FScene * Scene)
	{
		if (HasMetaFlag(MetaFlag_SceneTileMetaDirty))
		{
			SCENE_META_LOG(Log, "Tile meta dirty.\n");
			const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();
			// Collect visible scene tile instances, collect ECR_INTERSECT tile in front.
			// Sort by visible info
			SortedTilePositions.clear();
			SceneTileIntersected = 0;
			SceneTileInner = 0;
			for (const auto& T : SceneTileVisibleInfo)
			{
				const vector2di& TilePos = T.first;
				if (SceneTileVisibleInfo[TilePos] == ECR_INTERSECT)
				{
					SortedTilePositions.push_back(TilePos);
					++SceneTileIntersected;
				}
			}
			for (const auto& T : SceneTileVisibleInfo)
			{
				const vector2di& TilePos = T.first;
				if (SceneTileVisibleInfo[TilePos] == ECR_INSIDE)
				{
					SortedTilePositions.push_back(TilePos);
					++SceneTileInner;
				}
			}

			// Collect instances info, and remember meta infos
			SceneInstancesAdded = 0;
			SceneInstancesIntersected = 0;
			ScenePrimitivesAdded = 0;
			ActiveSceneTileInfoMap.clear();
			for (uint32 t = 0 ; t < (uint32)SortedTilePositions.size() ; ++ t)
			{
				const vector2di& TilePos = SortedTilePositions[t];
				FSceneTileResourcePtr TileRes = SceneTileResources.find(TilePos)->second;

				// Get available Instances range of this tile
				const uint32 InstancesStartIndex = SceneInstancesAdded;
				SceneInstancesAdded += TileRes->GetInstanceBuffer()->GetInstancesCount();
				if (t < SceneTileIntersected)
				{
					SceneInstancesIntersected += TileRes->GetInstanceBuffer()->GetInstancesCount();
				}

				// Get available Primitive range of this tile
				const uint32 PrimitiveStartIndex = ScenePrimitivesAdded;
				ScenePrimitivesAdded += TileRes->GetTotalMeshSections();
				TI_ASSERT(TileRes->GetTotalMeshSections() == TileRes->GetInstanceCountAndOffset().size());

				// Remember the offset of this tile
				TI_ASSERT(ActiveSceneTileInfoMap.find(TilePos) == ActiveSceneTileInfoMap.end());
				FUInt4 PrimInfo(
					TileRes->GetTotalMeshSections(),
					PrimitiveStartIndex,
					TileRes->GetInstanceBuffer()->GetInstancesCount(),
					InstancesStartIndex);
				ActiveSceneTileInfoMap[TilePos] = PrimInfo;
			}

			// Create primitive bbox buffer
			ScenePrimitiveBBoxes = ti_new FScenePrimitiveBBoxes(ScenePrimitivesAdded);

			// Collect Instances
			SceneInstancesMetaInfo = ti_new FSceneInstanceMetaInfo(SceneInstancesAdded);
			for (const auto& TilePos : SortedTilePositions)
			{
				FSceneTileResourcePtr TileRes = SceneTileResources.find(TilePos)->second;

				const FUInt4& TilePrimInfo = ActiveSceneTileInfoMap[TilePos];
				const uint32 PrimitiveStartIndex = TilePrimInfo.Y;
				const uint32 InstancesStartIndex = TilePrimInfo.W;

				// Collect instance meta info data.
				const TVector<vector2di>& InstancesCountAndOffset = TileRes->GetInstanceCountAndOffset();
				uint32 InstanceIndex = InstancesStartIndex;
				for (uint32 i = 0; i < (uint32)InstancesCountAndOffset.size(); ++i)
				{
					uint32 PrimitiveIndex = i + PrimitiveStartIndex;
					for (int32 c = 0; c < InstancesCountAndOffset[i].X; ++c)
					{
						uint32 Index = InstanceIndex + c;
						// Primitive index this instance link to, in order to get primitive bbox
						SceneInstancesMetaInfo->UniformBufferData[Index].Info.X = PrimitiveIndex;
						// Mark as loading, change this when primitive loaded
						SceneInstancesMetaInfo->UniformBufferData[Index].Info.W = 0;
					}
					InstanceIndex += InstancesCountAndOffset[i].X;
					TI_ASSERT(InstanceIndex <= SceneInstancesAdded);
					SceneMetaFlags |= MetaFlag_SceneInstanceMetaDirty;
				}
			}
		}
		if (Scene->HasSceneFlag(FScene::ScenePrimitivesDirty) ||
			HasMetaFlag(MetaFlag_SceneInstanceMetaDirty))
		{
			SCENE_META_LOG(Log, "Primitive dirty.\n");
			// Update scene primitive meta infos by scene order
			const TVector<FPrimitivePtr>& Primitives = Scene->GetStaticDrawList(LIST_OPAQUE);
			const THMap<vector2di, FSceneTileResourcePtr>& SceneTiles = Scene->GetSceneTiles();
			
			uint32 PrimitiveIndexInDrawList = 0;
			for (auto P : Primitives)
			{
				const vector2di& TilePos = P->GetSceneTilePos();
				if (SceneTileVisibleInfo[TilePos] == ECR_OUTSIDE)
				{
					continue;
				}
				uint32 IndexInTile = P->GetIndexInSceneTile();
				const aabbox3df& BBox = P->GetBBox();

				TI_ASSERT(ActiveSceneTileInfoMap.find(TilePos) != ActiveSceneTileInfoMap.end());
				const FUInt4& PosInfo = ActiveSceneTileInfoMap[TilePos];

				const uint32 PrimitivesCount = PosInfo.X;
				const uint32 PrimitivesStartIndex = PosInfo.Y;
				const uint32 InstancesCount = PosInfo.Z;
				const uint32 InstancesStartIndex = PosInfo.W;

				// Mark a global instance offset for primitive in this merged instance buffer
				P->SetGlobalInstanceOffset(InstancesStartIndex);

				const uint32 PrimitiveIndex = PrimitivesStartIndex + IndexInTile;
				// Update primitive bboxes in meta data
				ScenePrimitiveBBoxes->UniformBufferData[PrimitiveIndex].MinEdge = FFloat4(BBox.MinEdge.X, BBox.MinEdge.Y, BBox.MinEdge.Z, 0);
				ScenePrimitiveBBoxes->UniformBufferData[PrimitiveIndex].MaxEdge = FFloat4(BBox.MaxEdge.X, BBox.MaxEdge.Y, BBox.MaxEdge.Z, 0);
				SceneMetaFlags |= MetaFlag_ScenePrimitiveMetaDirty;

				// Update instance meta data Info.W as loaded (1)
				FSceneTileResourcePtr SceneTile = SceneTiles.find(TilePos)->second;
				const vector2di& InstancesCountAndOffset = SceneTile->GetInstanceCountAndOffset()[IndexInTile];
				const uint32 PrimitiveInstanceStart = InstancesCountAndOffset.Y + InstancesStartIndex;
				const uint32 PrimitiveInstanceEnd = PrimitiveInstanceStart + InstancesCountAndOffset.X;
				for (uint32 i = PrimitiveInstanceStart; i < PrimitiveInstanceEnd; ++i)
				{
					SceneInstancesMetaInfo->UniformBufferData[i].Info.Y = PrimitiveIndexInDrawList;
					SceneInstancesMetaInfo->UniformBufferData[i].Info.W = 1;
				}
				++PrimitiveIndexInDrawList;
				SceneMetaFlags |= MetaFlag_SceneInstanceMetaDirty;
			}
		}

		UpdateGPUResources();
	}

	void FSceneMetaInfos::CollectInstanceBuffers(FScene * Scene)
	{
		if (HasMetaFlag(MetaFlag_SceneTileMetaDirty))
		{
			FRHI * RHI = FRHI::Get();

			// Merge all instance buffers from scene tile node into a BIG one.
			const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();
			const uint32 TilesCount = (uint32)SceneTileResources.size();

			// SceneInstancesAdded calculated in CollectSceneMetaInfos().
			const uint32 TotalInstances = SceneInstancesAdded;
			if (TotalInstances > 0)
			{
				MergedInstanceBuffer = RHI->CreateEmptyInstanceBuffer(TotalInstances, TInstanceBuffer::InstanceStride);
#if defined (TIX_DEBUG)
				MergedInstanceBuffer->SetResourceName("SceneMergedIB");
#endif
				RHI->UpdateHardwareResourceIB(MergedInstanceBuffer, nullptr);

				uint32 InstanceDstOffset = 0;
				for (const auto& TilePos : SortedTilePositions)
				{
					FSceneTileResourcePtr TileRes = SceneTileResources.find(TilePos)->second;
					FInstanceBufferPtr TileInstances = TileRes->GetInstanceBuffer();

					RHI->CopyBufferRegion(MergedInstanceBuffer, InstanceDstOffset, TileInstances, 0, TileInstances->GetInstancesCount());
					InstanceDstOffset += TileInstances->GetInstancesCount();
				}
				TI_ASSERT(InstanceDstOffset == TotalInstances);

			}
		}
	}

	void FSceneMetaInfos::UpdateGPUResources()
	{
		if (HasMetaFlag(MetaFlag_ScenePrimitiveMetaDirty))
		{
			ScenePrimitiveBBoxes->InitUniformBuffer();
		}
		if (HasMetaFlag(MetaFlag_SceneInstanceMetaDirty))
		{
			SceneInstancesMetaInfo->InitUniformBuffer();
		}
	}

	void FSceneMetaInfos::ClearMetaFlags()
	{
		SceneMetaFlags = 0;
	}
}
