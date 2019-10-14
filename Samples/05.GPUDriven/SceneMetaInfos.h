/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	// MinEdge.w is 1 means this tile info is valid, 0 for empty data
	#define MAX_SCENE_TILE_META_NUM (256)
	BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY(FSceneTileMetaInfo, MAX_SCENE_TILE_META_NUM)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MinEdge)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MaxEdge)
	END_UNIFORM_BUFFER_STRUCT(FSceneTileMetaInfo)

	// Primitive BBox info
	#define MAX_STATIC_MESH_IN_SCENE (2048)
	BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY(FScenePrimitiveBBoxes, MAX_STATIC_MESH_IN_SCENE)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MinEdge)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MaxEdge)
	END_UNIFORM_BUFFER_STRUCT(FScenePrimitiveBBoxes)

	// Info.x = primitive index this instance link to
	// Info.y = scene tile index this instance belongs to
	// Info.w = if this primitive is loaded. 1 = loaded; 0 = loading
	#define MAX_INSTANCES_IN_SCENE (40 * 1024)
	BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY(FSceneInstanceMetaInfo, MAX_INSTANCES_IN_SCENE)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FUInt4, Info)
	END_UNIFORM_BUFFER_STRUCT(FSceneInstanceMetaInfo)

	class FSceneMetaInfos
	{
	public:
		FSceneMetaInfos();
		~FSceneMetaInfos();

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
		FUniformBufferPtr GetTileMetaUniform()
		{
			return SceneTileMetaInfo->UniformBuffer;
		}
		FUniformBufferPtr GetPrimitiveBBoxesUniform()
		{
			return ScenePrimitiveBBoxes->UniformBuffer;
		}
		FUniformBufferPtr GetInstanceMetaUniform()
		{
			return SceneInstancesMetaInfo->UniformBuffer;
		}
		FInstanceBufferPtr GetMergedInstanceBuffer()
		{
			return MergedInstanceBuffer;
		}

	private:
		void Init();
		void UpdateGPUResources();


	private:
		uint32 SceneMetaFlags;

		// Scene tile meta info
		FSceneTileMetaInfoPtr SceneTileMetaInfo;
		uint32 ScenePrimitivesAdded;
		uint32 SceneInstancesAdded;
		// Scene tile primitive count infos map.
		// Key is tile position, value is {PritmivesCount, PrimitivesOffset in ScenePrimitiveBBox, InstancesCount, InstancesOffset in SceneInstancesMetaInfo};
		THMap<vector2di, FUInt4> SceneTileInfoMap;

		// Scene Instances info
		FSceneInstanceMetaInfoPtr SceneInstancesMetaInfo;
		FInstanceBufferPtr MergedInstanceBuffer;

		// Scene primitive meta info
		FScenePrimitiveBBoxesPtr ScenePrimitiveBBoxes;

		friend class FScene;
	};
} // end namespace tix
