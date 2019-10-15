/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	//// MinEdge.w is 1 means this tile info is valid, 0 for empty data
	//#define MAX_SCENE_TILE_META_NUM (256)
	//BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY(FSceneTileMetaInfo, MAX_SCENE_TILE_META_NUM)
	//	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MinEdge)
	//	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MaxEdge)
	//END_UNIFORM_BUFFER_STRUCT(FSceneTileMetaInfo)

	// Primitive BBox info
	//#define MAX_STATIC_MESH_IN_SCENE (2048)
	BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY_DYNAMIC(FSceneStaticMeshBBoxes)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MinEdge)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MaxEdge)
	END_UNIFORM_BUFFER_STRUCT(FSceneStaticMeshBBoxes)

	// Info.x = primitive index this instance link to
	// Info.w = if this primitive is loaded. 1 = loaded; 0 = loading
	//#define MAX_INSTANCES_IN_SCENE (40 * 1024)
	BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY_DYNAMIC(FSceneInstanceMetaInfo)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FUInt4, Info)
	END_UNIFORM_BUFFER_STRUCT(FSceneInstanceMetaInfo)

	class FSceneMetaInfos
	{
	public:
		FSceneMetaInfos();
		~FSceneMetaInfos();

		void DoSceneTileCulling(FScene * Scene, const SViewFrustum& ViewFrustum);
		void CollectSceneMetaInfos(FScene * Scene);
		void CollectInstanceBuffers(FScene * Scene);
		void ClearMetaFlags();

		enum FSceneMetaFlag
		{
			MetaFlag_SceneTileMetaDirty = 1 << 0,
			MetaFlag_ScenePrimitiveMetaDirty = 1 << 1,
			MetaFlag_SceneInstanceMetaDirty = 1 << 2,
		};
		bool HasMetaFlag(FSceneMetaFlag InMetaFlag) const
		{
			return (SceneMetaFlags & InMetaFlag) != 0;
		}
		FUniformBufferPtr GetPrimitiveBBoxesUniform()
		{
			return SceneStaticMeshBBoxes->UniformBuffer;
		}
		FUniformBufferPtr GetInstanceMetaUniform()
		{
			return SceneInstancesMetaInfo->UniformBuffer;
		}
		FInstanceBufferPtr GetMergedInstanceBuffer()
		{
			return MergedInstanceBuffer;
		}
		uint32 GetSceneStaticMeshAdded() const
		{
			return SceneStaticMeshAdded;
		}
		uint32 GetSceneInstancesAdded() const
		{
			return SceneInstancesAdded;
		}
		bool IsTileVisible(const vector2di& TilePos)
		{
			return SceneTileVisibleInfo[TilePos] != ECR_OUTSIDE;
		}

	private:
		void UpdateGPUResources();


	private:
		uint32 SceneMetaFlags;

		// Scene tile meta info
		//FSceneTileMetaInfoPtr SceneTileMetaInfo;
		THMap<vector2di, uint32> SceneTileVisibleInfo;
		
		// Static mesh count that added to scene from visible scene tiles
		// A static mesh can contain 1 or more primitives
		uint32 SceneStaticMeshAdded;
		// Instances Count that added to scene from visible scene tiles
		uint32 SceneInstancesAdded;

		TVector<vector2di> SortedTilePositions;

		// Scene tiles count that intersects with frustum
		uint32 SceneTileIntersected;
		// Scene tiles count that totally in frustum
		uint32 SceneTileInner;
		// Scene tile primitive count infos map.
		// Key is tile position, value is {PritmivesCount, PrimitivesOffset in ScenePrimitiveBBox, InstancesCount, InstancesOffset in SceneInstancesMetaInfo};
		THMap<vector2di, FUInt4> ActiveSceneTileInfoMap;

		// Scene Instances info
		FSceneInstanceMetaInfoPtr SceneInstancesMetaInfo;
		FInstanceBufferPtr MergedInstanceBuffer;

		// Scene primitive meta info
		FSceneStaticMeshBBoxesPtr SceneStaticMeshBBoxes;

		friend class FScene;
	};
} // end namespace tix
