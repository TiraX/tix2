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

	void FScene::AddNode(FNode * Node, FNode * Parent)
	{
		TI_ASSERT(IsRenderThread());
		Parent->AddChild(Node);
	}

	void FScene::RemoveNode(FNode * Node)
	{
		TI_ASSERT(IsRenderThread());
		Node->Remove();
		ti_delete Node;
	}
}
