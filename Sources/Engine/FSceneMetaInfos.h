/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	#define MAX_SCENE_TILE_META_NUM (256)
	BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY(FSceneTileMetaInfo, MAX_SCENE_TILE_META_NUM)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MinEdge)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MaxEdge)
	END_UNIFORM_BUFFER_STRUCT(FSceneTileMetaInfo)

	class FSceneMetaInfos
	{
	public:
		FSceneMetaInfos();
		~FSceneMetaInfos();

		void RegisterSceneTile(const vector2di& TilePos, const aabbox3df& TileBBox);
		void UpdateGPUResources();
		void ClearMetaFlags();

		enum FSceneMetaFlag
		{
			MetaFlag_SceneTileMetaDirty = 1 << 0,
		};
		bool HasMetaFlag(FSceneMetaFlag InMetaFlag) const
		{
			return (SceneMetaFlags & InMetaFlag) != 0;
		}
		FUniformBufferPtr GetTileMetaUniform()
		{
			return SceneTileMetaInfo->UniformBuffer;
		}

	private:
		void Init();


	private:
		uint32 SceneMetaFlags;

		// Scene tile meta index map, key is tile pos, value is the meta index in SceneTileMetaInfos
		THMap<vector2di, uint32> SceneTileMetaIndexMap;

		// Scene tile meta info
		FSceneTileMetaInfoPtr SceneTileMetaInfo;
		uint32 ActiveSceneTileInfos;

		friend class FScene;
	};
} // end namespace tix
