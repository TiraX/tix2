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

	TInstanceBufferPtr TSceneTileResource::GetInstanceBufferByIndex(int32 Index)
	{
		return MeshInstances[Index];
	}

	void TSceneTileResource::InitRenderThreadResource()
	{
		// Init all instances render resource
		for (auto Ins : MeshInstances)
		{
			Ins->InitRenderThreadResource();
		}
	}

	void TSceneTileResource::DestroyRenderThreadResource()
	{
		for (auto Ins : MeshInstances)
		{
			Ins->DestroyRenderThreadResource();
		}
	}

}