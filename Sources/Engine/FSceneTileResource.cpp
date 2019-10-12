/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FSceneTileResource.h"

namespace tix
{
	FSceneTileResource::FSceneTileResource()
		: FRenderResource(RRT_SCENE_TILE)
	{
	}

	FSceneTileResource::FSceneTileResource(TSceneTileResourcePtr InSceneTileResource)
		: FRenderResource(RRT_SCENE_TILE)
	{
		TI_ASSERT(IsGameThread());
		Position = InSceneTileResource->Position;
		BBox = InSceneTileResource->BBox;
		InstanceCountAndOffset = InSceneTileResource->InstanceCountAndOffset;
		InstanceBuffer = InSceneTileResource->MeshInstanceBuffer->InstanceResource;
	}

	FSceneTileResource::~FSceneTileResource()
	{
	}
}