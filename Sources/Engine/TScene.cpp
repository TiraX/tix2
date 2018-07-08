/*
TiX Engine v2.0 Copyright (C) 2018
By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TScene.h"

namespace tix
{
	TNodeSceneRoot::TNodeSceneRoot()
		: TNode(ENT_ROOT, nullptr)
	{
	}
	TNodeSceneRoot::~TNodeSceneRoot()
	{
	}

	void TNodeSceneRoot::CreateRenderThreadNode()
	{
		TI_ASSERT(Node_RenderThread == nullptr);
		// Create render thread node and add it to FScene
		Node_RenderThread = ti_new FNode(ENT_ROOT, nullptr);
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(SetFSceneRootNode,
			FNode*, Node, Node_RenderThread,
			{
				FScene * scene = RenderThread->GetRenderScene();
				scene->SetRootNode(Node);
			});
	}
	////////////////////////////////////////////////////////////////////////////
	TScene::TScene()
		: Flag(0)
	{
		// Create root element
		NodeRoot = ti_new TNodeSceneRoot();
		NodeRoot->CreateRenderThreadNode();

		// Create default camera, this camera can only deleted by render stage.
		DefaultCamera = TNodeFactory::CreateNode<TNodeCamera>(nullptr);
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

	void TScene::TickAllNodes(float dt, TNode* root)
	{
		if (root == nullptr)
			root = NodeRoot;

		// Update all nodes logic
		root->Update(dt);

		// Update all nodes transformations
		root->UpdateAllTransformation();
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
}
