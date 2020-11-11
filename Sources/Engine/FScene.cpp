/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FScene.h"
#include "FSceneLights.h"

namespace tix
{
	FScene::FScene()
		: SceneLights(nullptr)
		, SceneFlags(0)
	{
		SceneLights = ti_new FSceneLights;

		// Reserve memory for containers
		SceneTiles.reserve(128);
	}

	FScene::~FScene()
	{
		SAFE_DELETE(SceneLights);
	}

	void FScene::SetViewProjection(const FViewProjectionInfo& Info)
	{
		TI_ASSERT(IsRenderThread());
		ViewProjection = Info;
		SetSceneFlag(ViewProjectionDirty);
	}

	void FScene::SetEnvironmentInfo(const FEnvironmentInfo& Info)
	{
		TI_ASSERT(IsRenderThread());
		EnvInfo = Info;
		SetSceneFlag(EnvironmentDirty);
	}

	void FScene::InitRenderFrame()
	{
		PrepareViewUniforms();
	}

	void FScene::PrepareViewUniforms()
	{
		if (HasSceneFlag(FScene::ViewUniformDirty))
		{
			// Always make a new View uniform buffer for on-the-fly rendering
			ViewUniformBuffer = ti_new FViewUniformBuffer();

			const FViewProjectionInfo& VPInfo = GetViewProjection();
			ViewUniformBuffer->UniformBufferData[0].ViewProjection = VPInfo.MatProj * VPInfo.MatView;
			ViewUniformBuffer->UniformBufferData[0].ViewDir = VPInfo.CamDir;
			ViewUniformBuffer->UniformBufferData[0].ViewPos = VPInfo.CamPos;
			ViewUniformBuffer->UniformBufferData[0].MainLightDirection = EnvInfo.MainLightDir;
			ViewUniformBuffer->UniformBufferData[0].MainLightColor = EnvInfo.MainLightColor * EnvInfo.MainLightIntensity;

			ViewUniformBuffer->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
		}
	}

	void FScene::AddSceneTileInfo(FSceneTileResourcePtr SceneTileResource)
	{
		if (SceneTileResource != nullptr)
		{
			SceneTiles[SceneTileResource->GetTilePosition()] = SceneTileResource;

			// Mark flag scene tile dirty
			SetSceneFlag(SceneTileDirty);
		}
	}

	void FScene::RemoveSceneTileInfo(FSceneTileResourcePtr SceneTileResource)
	{
		TI_TODO("Add scene tile unregister.");
		TI_ASSERT(0);
	}

	void FScene::AddSceneMeshBuffer(FMeshBufferPtr InMesh, FMeshBufferPtr InOccludeMesh, FUniformBufferPtr InClusterData)
	{
		THMap<FMeshBufferPtr, FSceneMeshInfo>::iterator It = SceneMeshes.find(InMesh);
		if (It != SceneMeshes.end())
		{
			TI_ASSERT(It->second.OccludeMesh == InOccludeMesh && It->second.ClusterData == InClusterData);
			++It->second.References;
		}
		else
		{
			SceneMeshes[InMesh] = FSceneMeshInfo(1, InOccludeMesh, InClusterData);
		}
	}

	void FScene::RemoveSceneMeshBuffer(FMeshBufferPtr InMesh)
	{
		THMap<FMeshBufferPtr, FSceneMeshInfo>::iterator It = SceneMeshes.find(InMesh);
		TI_ASSERT(It != SceneMeshes.end() && It->second.References > 0);

		--It->second.References;
		if (It->second.References == 0)
		{
			SceneMeshes.erase(It);
		}
	}

	void FScene::AddEnvLight(FEnvLightPtr InEnvLight)
	{
		TI_TODO("Create quad-tree to fast find nearest Env Light.");
		EnvLight = InEnvLight;
	}

	void FScene::RemoveEnvLight(FEnvLightPtr InEnvLight)
	{
		TI_ASSERT(0);
	}

}
