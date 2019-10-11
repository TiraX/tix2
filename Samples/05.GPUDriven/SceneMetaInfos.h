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
	#define MAX_DRAW_CALL_IN_SCENE (2048)
	BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY(FScenePrimitiveBBoxes, MAX_DRAW_CALL_IN_SCENE)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MinEdge)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MaxEdge)
	END_UNIFORM_BUFFER_STRUCT(FScenePrimitiveBBoxes)

	// Info.x = primitive index this instance link to
	// Info.y = scene tile index this instance belongs to
	#define MAX_INSTANCES_IN_SCENE (40 * 1024)
	BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY(FSceneInstanceMetaInfo, MAX_INSTANCES_IN_SCENE)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FUInt4, Info)
	END_UNIFORM_BUFFER_STRUCT(FSceneInstanceMetaInfo)

	class FSceneMetaInfos
	{
	public:
		FSceneMetaInfos();
		~FSceneMetaInfos();

		void UpdateSceneInfos(FScene * Scene);
		//virtual void OnAddStaticMeshPrimitives(const TVector<FPrimitivePtr>& InPrimitives) override;
		//virtual void OnAddSceneTile(TSceneTileResourcePtr InSceneTileRes) override;

		//void UpdateGPUResources();
		//void ClearMetaFlags();

		//enum FSceneMetaFlag
		//{
		//	MetaFlag_SceneTileMetaDirty = 1 << 0,
		//	MetaFlag_ScenePrimitiveMetaDirty = 1 << 1,
		//	MetaFlag_SceneInstanceMetaDirty = 1 << 2,
		//};
		//bool HasMetaFlag(FSceneMetaFlag InMetaFlag) const
		//{
		//	return (SceneMetaFlags & InMetaFlag) != 0;
		//}
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

	private:
		void Init();


	private:
		//uint32 SceneMetaFlags;

		// Scene tile meta info
		FSceneTileMetaInfoPtr SceneTileMetaInfo;
		uint32 ActiveSceneTileInfos;
		uint32 ScenePrimitivesAdded;

		// Scene Instances info
		FSceneInstanceMetaInfoPtr SceneInstancesMetaInfo;
		uint32 ActiveSceneInstanceInfos;

		// Scene primitive meta info
		FScenePrimitiveBBoxesPtr ScenePrimitiveBBoxes;
		uint32 ActiveScenePrimitiveBBoxes;

		friend class FScene;
	};
} // end namespace tix
