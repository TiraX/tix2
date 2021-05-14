/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TSceneTileResource.h"

namespace tix
{
	TSceneTileResource::TSceneTileResource()
		: TResource(ERES_SCENE_TILE)
		, TotalEnvLights(0)
		, TotalStaticMeshes(0)
		, TotalSMSections(0)
		, TotalSMInstances(0)
		, TotalSKMActors(0)
	{
	}

	TSceneTileResource::~TSceneTileResource()
	{
	}

	void TSceneTileResource::InitRenderThreadResource()
	{
		// Init all instances render resource
		TI_ASSERT(SMInstanceBuffer != nullptr);
		SMInstanceBuffer->InitRenderThreadResource();

		// Register scene tile info to FSceneMetaInfos, for GPU tile frustum cull
		TI_ASSERT(RenderThreadTileResource == nullptr);
		RenderThreadTileResource = ti_new FSceneTileResource(*this);
		FSceneTileResourcePtr SceneTileRes = RenderThreadTileResource;
		ENQUEUE_RENDER_COMMAND(AddSceneTileRes)(
			[SceneTileRes]()
			{
				FRenderThread::Get()->GetRenderScene()->AddSceneTileInfo(SceneTileRes);
			});
	}

	void TSceneTileResource::DestroyRenderThreadResource()
	{
		SMInstanceBuffer->DestroyRenderThreadResource();

		TI_ASSERT(RenderThreadTileResource != nullptr);
		FSceneTileResourcePtr SceneTileResource = RenderThreadTileResource;
		ENQUEUE_RENDER_COMMAND(RemoveSceneTileRes)(
			[SceneTileResource]()
			{
				//SceneTileResource = nullptr;
			});
		RenderThreadTileResource = nullptr;
	}
}