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

	#define MAX_DRAW_CALL_IN_SCENE (2048)
	BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY(FScenePrimitiveMetaInfo, MAX_DRAW_CALL_IN_SCENE)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FUInt4, Info)
	END_UNIFORM_BUFFER_STRUCT(FScenePrimitiveMetaInfo)

	class FSceneMetaInfos : public FSceneDelegate
	{
	public:
		FSceneMetaInfos();
		virtual ~FSceneMetaInfos();

		virtual void OnAddPrimitive(FPrimitivePtr InPrimitive) override;
		virtual void OnAddSceneTile(TSceneTileResourcePtr InSceneTileRes) override;

		void RegisterSceneTile(const vector2di& TilePos, const aabbox3df& TileBBox);
		void RegisterPrimitive(FPrimitivePtr InPrimitive);
		void UpdateGPUResources();
		void ClearMetaFlags();

		enum FSceneMetaFlag
		{
			MetaFlag_SceneTileMetaDirty = 1 << 0,
			MetaFlag_ScenePrimitiveMetaDirty = 1 << 1,
		};
		bool HasMetaFlag(FSceneMetaFlag InMetaFlag) const
		{
			return (SceneMetaFlags & InMetaFlag) != 0;
		}
		FUniformBufferPtr GetTileMetaUniform()
		{
			return SceneTileMetaInfo->UniformBuffer;
		}
		FUniformBufferPtr GetPrimitiveMetaUniform()
		{
			return ScenePrimitiveMetaInfo->UniformBuffer;
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

		// Scene primitive meta info
		FScenePrimitiveMetaInfoPtr ScenePrimitiveMetaInfo;
		uint32 ActiveScenePrimitiveInfos;

		friend class FScene;
	};
} // end namespace tix
