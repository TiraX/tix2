/*
	TiX Engine v2.0 Copyright (C) 2018~2021
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
			const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();
			
			for (uint32 t = 0; t < (uint32)SortedTilePositions.size(); ++t)
			{
				const vector2di& TilePos = SortedTilePositions[t];
				if (SceneTileVisibleInfo[TilePos] == ECR_OUTSIDE)
				{
					continue;
				}
				FSceneTileResourcePtr TileRes = SceneTileResources.find(TilePos)->second;

				TI_ASSERT(ActiveSceneTileInfoMap.find(TilePos) != ActiveSceneTileInfoMap.end());
				const FUInt4& PosInfo = ActiveSceneTileInfoMap[TilePos];
				const uint32 PrimitivesCount = PosInfo.X;
				const uint32 PrimitivesStartIndex = PosInfo.Y;
				const uint32 InstancesCount = PosInfo.Z;
				const uint32 InstancesStartIndex = PosInfo.W;

				const TVector<FPrimitivePtr>& TilePrimitives = TileRes->GetPrimitives();
				for (uint32 PIndex = 0 ; PIndex < (uint32)TilePrimitives.size(); ++PIndex)
				{
					FPrimitivePtr P = TilePrimitives[PIndex];

					if (P != nullptr)
					{
						const aabbox3df& BBox = P->GetBBox();

						// Mark a global instance offset for primitive in this merged instance buffer
						P->SetGlobalInstanceOffset(InstancesStartIndex);

						const uint32 PrimitiveIndex = PrimitivesStartIndex + PIndex;
						// Update primitive bboxes in meta data
						ScenePrimitiveBBoxes->UniformBufferData[PrimitiveIndex].MinEdge = FFloat4(BBox.MinEdge.X, BBox.MinEdge.Y, BBox.MinEdge.Z, 0);
						ScenePrimitiveBBoxes->UniformBufferData[PrimitiveIndex].MaxEdge = FFloat4(BBox.MaxEdge.X, BBox.MaxEdge.Y, BBox.MaxEdge.Z, 0);
						SceneMetaFlags |= MetaFlag_ScenePrimitiveMetaDirty;
					}

					// Update instance meta data Info.W as loaded (1)
					const vector2di& InstancesCountAndOffset = TileRes->GetInstanceCountAndOffset()[PIndex];
					const uint32 PrimitiveInstanceStart = InstancesCountAndOffset.Y + InstancesStartIndex;
					const uint32 PrimitiveInstanceEnd = PrimitiveInstanceStart + InstancesCountAndOffset.X;
					for (uint32 i = PrimitiveInstanceStart; i < PrimitiveInstanceEnd; ++i)
					{
						SceneInstancesMetaInfo->UniformBufferData[i].Info.W = (P != nullptr) ? 1 : 0;
					}
					SceneMetaFlags |= MetaFlag_SceneInstanceMetaDirty;

				}
			}
		}

		if (Scene->HasSceneFlag(FScene::ScenePrimitivesDirty))
		{
			// There is primitives changed, mark cluster data as dirty.
			// Need to collect all cluster data into one big buffer.
			SceneMetaFlags |= MetaFlag_SceneClusterMetaDirty;

			// Same with above, collect all vertex data and index data into a big buffer
			SceneMetaFlags |= MetaFlag_SceneMeshBufferDirty;
		}
	}

	void FSceneMetaInfos::CollectInstanceBuffers(FScene * Scene)
	{
		if (HasMetaFlag(MetaFlag_SceneTileMetaDirty))
		{
			FRHI * RHI = FRHI::Get();

			// Merge all instance buffers from scene tile node into a BIG one.
			const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();

			// SceneInstancesAdded calculated in CollectSceneMetaInfos().
			const uint32 TotalInstances = SceneInstancesAdded;
			if (TotalInstances > 0)
			{
				MergedInstanceBuffer = RHI->CreateEmptyInstanceBuffer(TotalInstances, TInstanceBuffer::InstanceStride);
				MergedInstanceBuffer->SetResourceName("SceneMergedIB");
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

	void FSceneMetaInfos::CollectMeshBuffers(FScene * Scene)
	{
		if (HasMetaFlag(MetaFlag_SceneMeshBufferDirty))
		{
			SCENE_META_LOG(Log, "Scene mesh buffer dirty.\n");
			FRHI * RHI = FRHI::Get();

			const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();

			// Calculate total size of vertex and index
			uint32 TotalMeshCount, TotalVertexCount, TotalIndexCount;
			TotalMeshCount = 0;
			TotalVertexCount = 0;
			TotalIndexCount = 0;
			uint32 VBFormat = 0;
			for (const auto& TilePos : SortedTilePositions)
			{
				FSceneTileResourcePtr TileRes = SceneTileResources.find(TilePos)->second;
				for (auto Prim : TileRes->GetPrimitives())
				{
					if (Prim != nullptr)
					{
						FMeshBufferPtr MeshBuffer = Prim->GetMeshBuffer();
						TotalVertexCount += MeshBuffer->GetVerticesCount();
						TotalIndexCount += MeshBuffer->GetIndicesCount();
						TI_ASSERT(MeshBuffer->GetIndexType() == EIT_32BIT);	// Compute shader can only access 32bit aligned data
						TI_ASSERT(VBFormat == 0 || VBFormat == MeshBuffer->GetVSFormat());
						VBFormat = MeshBuffer->GetVSFormat();
						++TotalMeshCount;
					}
				}
			}
			TI_ASSERT(TotalVertexCount > 0 && TotalIndexCount > 0);
			TI_TODO("Collected mesh buffer may be duplicated. Remove duplicated mesh. Copy a unique ONE.");

			// Copy all data into one big buffer
			const uint32 VertexStride = TMeshBuffer::GetStrideFromFormat(VBFormat);
			MergedMeshBuffer = RHI->CreateEmptyMeshBuffer(EPT_TRIANGLELIST, VBFormat, TotalVertexCount, EIT_32BIT, TotalIndexCount);
			MergedMeshBuffer->SetResourceName("MergedMeshBuffer");
			RHI->UpdateHardwareResourceMesh(MergedMeshBuffer, TotalVertexCount * VertexStride, VertexStride, TotalIndexCount * sizeof(uint32), EIT_32BIT, "MergedMeshBuffer");

			MergedSceneMeshBufferInfo = ti_new FSceneMeshBufferInfo(TotalMeshCount);

			uint32 VBOffset = 0, IBOffset = 0;
			uint32 PrimIndex = 0;
			for (const auto& TilePos : SortedTilePositions)
			{
				FSceneTileResourcePtr TileRes = SceneTileResources.find(TilePos)->second;
				for (auto Prim : TileRes->GetPrimitives())
				{
					if (Prim != nullptr)
					{
						FMeshBufferPtr MeshBuffer = Prim->GetMeshBuffer();
						RHI->SetResourceStateMB(MeshBuffer, RESOURCE_STATE_COPY_SOURCE);

						RHI->CopyBufferRegion(
							MergedMeshBuffer, 
							VBOffset, 
							IBOffset, 
							MeshBuffer, 
							0, 
							MeshBuffer->GetVerticesCount() * VertexStride, 
							0, 
							MeshBuffer->GetIndicesCount() * sizeof(uint32)); 

						MergedSceneMeshBufferInfo->UniformBufferData[PrimIndex].Info.X = VBOffset / sizeof(float);
						MergedSceneMeshBufferInfo->UniformBufferData[PrimIndex].Info.Y = IBOffset / sizeof(uint32);

						VBOffset += MeshBuffer->GetVerticesCount() * VertexStride;
						IBOffset += MeshBuffer->GetIndicesCount() * sizeof(uint32);

						++PrimIndex;
					}
				}
			}
			MergedSceneMeshBufferInfo->InitUniformBuffer();
			TI_ASSERT(PrimIndex == TotalMeshCount);
		}
	}

	void FSceneMetaInfos::CollectClusterMetaBuffers(FScene * Scene)
	{
		// Collect all mesh cluster infos in Scene
		if (HasMetaFlag(MetaFlag_SceneClusterMetaDirty))
		{
			SCENE_META_LOG(Log, "Cluster meta dirty.\n");
			FRHI * RHI = FRHI::Get();

			const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();

			// Calc total size of clusters
			const uint32 ClusterDataSize = sizeof(TMeshBuffer::TMeshClusterData);

			uint32 TotalClusterMetas = 0;
			for (const auto& TilePos : SortedTilePositions)
			{
				FSceneTileResourcePtr TileRes = SceneTileResources.find(TilePos)->second;
				for (auto Prim : TileRes->GetPrimitives())
				{
					TotalClusterMetas += Prim->GetClusterData()->GetElements();
				}
			}
			TI_ASSERT(TotalClusterMetas > 0);

			MergedClusterBoundingData = RHI->CreateUniformBuffer(ClusterDataSize, TotalClusterMetas);
			MergedClusterBoundingData->SetResourceName("MergedClusterData");
			RHI->UpdateHardwareResourceUB(MergedClusterBoundingData, nullptr);

			ClusterMetaData = ti_new FClusterMetaInfo(TotalClusterMetas);

			uint32 ElementOffset = 0;
			uint32 PrimIndex = 0;
			for (const auto& TilePos : SortedTilePositions)
			{
				FSceneTileResourcePtr TileRes = SceneTileResources.find(TilePos)->second;
				for (auto Prim : TileRes->GetPrimitives())
				{
					if (Prim != nullptr)
					{
						uint32 ClusterOffset = ElementOffset;
						uint32 ClusterCount = Prim->GetClusterData()->GetElements();
						
						RHI->CopyBufferRegion(MergedClusterBoundingData, ElementOffset * ClusterDataSize, Prim->GetClusterData(), ClusterCount * ClusterDataSize);

						for (uint32 c = 0 ; c < ClusterCount ; ++ c)
						{
							ClusterMetaData->UniformBufferData[ElementOffset + c].Info.X = PrimIndex;
						}

						++PrimIndex;
						ElementOffset += Prim->GetClusterData()->GetElements();
					}
				}
			}
			ClusterMetaData->InitUniformBuffer();
			TI_ASSERT(ElementOffset == TotalClusterMetas);
		}
		if (HasMetaFlag(MetaFlag_SceneClusterMetaDirty) ||
			HasMetaFlag(MetaFlag_SceneInstanceMetaDirty))
		{
			const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();

			// Mark cluster meta info into instance meta infos.
			uint32 ClusterOffset = 0;
			for (const auto& TilePos : SortedTilePositions)
			{
				FSceneTileResourcePtr TileRes = SceneTileResources.find(TilePos)->second;
				for (auto Prim : TileRes->GetPrimitives())
				{
					if (Prim != nullptr)
					{
						uint32 InstanceOffset = Prim->GetGlobalInstanceOffset();
						uint32 InstanceCount = Prim->GetInstanceCount();
						uint32 ClusterCount = Prim->GetClusterData()->GetElements();
						TI_ASSERT(ClusterCount > 0);
						for (uint32 Ins = 0; Ins < InstanceCount; ++Ins)
						{
							uint32 Index = InstanceOffset + Ins;
							// Info.y = cluster index begin
							// Info.z = cluster count
							SceneInstancesMetaInfo->UniformBufferData[Index].Info.Y = ClusterOffset;
							SceneInstancesMetaInfo->UniformBufferData[Index].Info.Z = ClusterCount;
						}

						ClusterOffset += Prim->GetClusterData()->GetElements();
					}
				}
			}
		}
	}

	void FSceneMetaInfos::UpdateGPUResources()
	{
		if (HasMetaFlag(MetaFlag_ScenePrimitiveMetaDirty))
		{
			ScenePrimitiveBBoxes->InitUniformBuffer();
		}
		if (HasMetaFlag(MetaFlag_SceneInstanceMetaDirty) ||
			HasMetaFlag(MetaFlag_SceneClusterMetaDirty))
		{
			SceneInstancesMetaInfo->InitUniformBuffer();
		}
	}

	void FSceneMetaInfos::ClearMetaFlags()
	{
		SceneMetaFlags = 0;
	}
}
