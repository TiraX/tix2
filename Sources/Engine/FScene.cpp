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
		SetSceneFlag(ViewProjectionDirty, true);
	}

	void FScene::AddPrimitive(FPrimitivePtr InPrimitive)
	{
		TI_ASSERT(IsRenderThread());
		StaticDrawList.push_back(InPrimitive);
	}

	void FScene::RemovePrimitive(FPrimitivePtr InPrimitive)
	{
		TI_ASSERT(IsRenderThread());
		TI_TODO("Find a fast way to locate Primitive in draw list.");

		TVector<FPrimitivePtr>::iterator it = tix_find(StaticDrawList.begin(), StaticDrawList.end(), InPrimitive);
		if (it != StaticDrawList.end())
		{
			StaticDrawList.erase(it);
		}
	}
}
