/*
TiX Engine v2.0 Copyright (C) 2018
By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TScene.h"

namespace tix
{
	TScene::TScene()
		: Flag(0)
	{
		// Create root element
		NodeRoot = ti_new TNodeSceneRoot();

		// Create default camera, this camera can only deleted by render stage.
		DefaultCamera = ti_new TNodeCamera(nullptr);
		DefaultCamera->SetAspectRatio(1.f);
		SetActiveCamera(DefaultCamera);
	}

	TScene::~TScene()
	{
		DefaultCamera->Remove();
		ti_delete	DefaultCamera;

		// remove root last
		ti_delete	NodeRoot;
	}

	void TScene::UpdateAll(float dt, TNode* root)
	{
		if (root == nullptr)
			root = NodeRoot;

		root->Update(dt);
	}

	void TScene::DrawAll(TNode* root)
	{
	}

	void TScene::SetActiveCamera(TNodeCamera* camera)
	{
		if (camera)
			ActiveCamera = camera;
		else
			ActiveCamera = DefaultCamera;
	}

	TNodeCamera* TScene::GetActiveCamera()
	{
		return ActiveCamera;
	}

	TNode* TScene::CreateNode(const char* node_id)
	{
		TNode* node = ti_new TNode(ENT_NODE, nullptr);
		node->SetId(node_id);
		return node;
	}

	TNodeCamera* TScene::CreateCamera()
	{
		return ti_new TNodeCamera(nullptr);
	}
}
