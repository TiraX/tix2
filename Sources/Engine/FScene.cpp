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

	void FScene::CollectAllMeshRelevance()
	{
		TI_TODO("Temp function, remove after refactor.");
		StaticDrawList.clear();

		TranverseNode(RootNode);
	}

	void FScene::TranverseNode(FNode * Node)
	{
		TI_TODO("Temp function, remove after refactor.");
		if (Node->GetType() == ENT_StaticMesh)
		{
			FNodeStaticMesh * StaticMesh = static_cast<FNodeStaticMesh*>(Node);
			StaticMesh->AddToStaticMeshList(StaticDrawList);
		}

		for (int32 c = 0; c < (int32)Node->GetChildrenCount(); ++c)
		{
			FNode * Child = Node->GetChild(c);
			TranverseNode(Child);
		}
	}

	void FScene::SetViewProjection(const FViewProjectionInfo& Info)
	{
		ViewProjection = Info;
	}
}
