/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FScene.h"

namespace tix
{
	FScene::FScene()
		: RootNode(nullptr)
		, SceneFlags(0)
	{
	}

	FScene::~FScene()
	{
	}

	void FScene::SetRootNode(FNode * Node)
	{
		TI_ASSERT(RootNode == nullptr);
		RootNode = Node;
	}

	void FScene::SetViewProjection(const FViewProjectionInfo& Info)
	{
		TI_ASSERT(IsRenderThread());
		ViewProjection = Info;
		SetSceneFlag(ViewProjectionDirty, true);
	}
}
