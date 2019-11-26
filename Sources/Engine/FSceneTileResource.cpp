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
		, TotalMeshes(0)
		, TotalMeshSections(0)
		, TotalInstances(0)
	{
		TI_ASSERT(0);
	}

	FSceneTileResource::FSceneTileResource(const TSceneTileResource& InSceneTileResource)
		: FRenderResource(RRT_SCENE_TILE)
		, Position(InSceneTileResource.Position)
		, BBox(InSceneTileResource.BBox)
		, TotalMeshes(InSceneTileResource.TotalMeshes)
		, TotalMeshSections(InSceneTileResource.TotalMeshSections)
		, TotalInstances(InSceneTileResource.TotalInstances)
		, InstanceCountAndOffset(InSceneTileResource.InstanceCountAndOffset)
	{
		TI_ASSERT(IsGameThread());
		InstanceBuffer = InSceneTileResource.MeshInstanceBuffer->InstanceResource;

		Primitives.resize(TotalMeshSections);
	}

	FSceneTileResource::~FSceneTileResource()
	{
	}
}