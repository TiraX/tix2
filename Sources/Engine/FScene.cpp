/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FScene.h"

namespace tix
{
	FScene::FScene()
		: SceneFlags(0)
	{
	}

	FScene::~FScene()
	{
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
}
