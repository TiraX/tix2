/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FScene.h"
#include "FSceneLights.h"

namespace tix
{
	FScene::FScene()
		: SceneFlags(0)
	{
		SceneLights = ti_new FSceneLights;
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

	void FScene::AddPrimitives(const TVector<FPrimitivePtr>& InPrimitives)
	{
		// Send different primitives to different draw list
		TI_ASSERT(IsRenderThread());
		// Combine all mesh buffers into a big buffer collection
		for (auto& P : InPrimitives)
		{
			// Allocate virtual texture position for this primitive
			FVTSystem::Get()->AllocatePositionForPrimitive(P);

			// Add to draw list
			StaticDrawLists[P->GetDrawList()].push_back(P);

			// Register meta info
			MetaInfos.RegisterPrimitive(P);

			// Mark flag primitives dirty
			SetSceneFlag(ScenePrimitivesDirty);
		}
	}

	void FScene::RemovePrimitives(const TVector<FPrimitivePtr>& InPrimitives)
	{
		TI_ASSERT(IsRenderThread());
		TI_TODO("Find a fast way to locate Primitive in draw list.");

		for (auto& P : InPrimitives)
		{
			// Remove from draw list
			TVector<FPrimitivePtr>& DrawList = StaticDrawLists[P->GetDrawList()];
			TVector<FPrimitivePtr>::iterator it = tix_find(DrawList.begin(), DrawList.end(), P);
			if (it != DrawList.end())
			{
				DrawList.erase(it);
			}

			// Remove from virtual texture
			FVTSystem::Get()->RemovePositionForPrimitive(P);

			TI_TODO("Unregister primitive from scene meta infos");

			// Mark flag primitives dirty
			SetSceneFlag(ScenePrimitivesDirty);
		}
	}

	void FScene::InitRenderFrame()
	{
		PrepareViewUniforms();
		PrepareMetaInfos();
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

	void FScene::PrepareMetaInfos()
	{
		MetaInfos.UpdateGPUResources();
	}

	void FScene::RegisterSceneTileMetaInfo(TSceneTileResourcePtr SceneTileResource)
	{
		MetaInfos.RegisterSceneTile(SceneTileResource->Position, SceneTileResource->BBox);
	}

	void FScene::UnregisterSceneTileMetaInfo()
	{
		TI_TODO("Add scene tile unregister.");
		TI_ASSERT(0);
	}
}
