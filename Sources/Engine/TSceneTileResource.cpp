/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TSceneTileResource.h"

namespace tix
{
	TSceneTileResource::TSceneTileResource()
		: TResource(ERES_SCENE_TILE)
	{
	}

	TSceneTileResource::~TSceneTileResource()
	{
	}

	void TSceneTileResource::InitRenderThreadResource()
	{
		// Init all instances render resource
		TI_ASSERT(MeshInstanceBuffer != nullptr);
		MeshInstanceBuffer->InitRenderThreadResource();
	}

	void TSceneTileResource::DestroyRenderThreadResource()
	{
		MeshInstanceBuffer->DestroyRenderThreadResource();
	}
}