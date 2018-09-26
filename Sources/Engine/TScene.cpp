/*
TiX Engine v2.0 Copyright (C) 2018
By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TScene.h"

namespace tix
{
	TNodeSceneRoot::TNodeSceneRoot(TNode * Parent)
		: TNode(TNodeSceneRoot::NODE_TYPE, nullptr)
	{
	}
	TNodeSceneRoot::~TNodeSceneRoot()
	{
	}
	////////////////////////////////////////////////////////////////////////////
	TScene::TScene()
		: Flag(0)
	{
		// Create root element
		NodeRoot = TNodeFactory::CreateNode<TNodeSceneRoot>(nullptr);

		// Create default camera, this camera can only deleted by render stage.
#ifdef TI_PLATFORM_WIN32
		DefaultCamera = TNodeFactory::CreateNode<TNodeCameraNav>(NodeRoot);
#else
		DefaultCamera = TNodeFactory::CreateNode<TNodeCamera>(NodeRoot);
#endif
		SetActiveCamera(DefaultCamera);
	}

	TScene::~TScene()
	{
		DefaultCamera->Remove();
		ti_delete	DefaultCamera;

		// remove root last
		ti_delete	NodeRoot;
	}

	void TScene::TickAllNodes(float Dt, TNode* Root)
	{
		if (Root == nullptr)
			Root = NodeRoot;

		Root->Tick(Dt);
	}

	void TScene::UpdateAllNodesTransforms(TNode* Root)
	{
		if (Root == nullptr)
			Root = NodeRoot;

		TI_TODO("Use parallel transform update.");
		Root->UpdateAllTransformation();

		BindLights();
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

	TNodeStaticMesh* TScene::AddStaticMesh(TMeshBufferPtr InMesh, TMaterialInstancePtr InMInstance, bool bCastShadow, bool bReceiveShadow)
	{
		// Create a static mesh node to hold mesh resource
		TNodeStaticMesh* StaticMesh = TNodeFactory::CreateNode<TNodeStaticMesh>(NodeRoot);

		// Link primitive to node
		StaticMesh->LinkMesh(InMesh, InMInstance, bCastShadow, bReceiveShadow);
		
		return StaticMesh;
	}

	TNodeLight* TScene::AddLight(float Intensity, const SColor& Color)
	{
		TNodeLight* Light = TNodeFactory::CreateNode<TNodeLight>(NodeRoot);
		Light->SetIntensity(Intensity);
		Light->SetColor(Color);
		Light->CreateFLight();
		return Light;
	}

	void TScene::ResetActiveLists()
	{
		for (int32 l = 0; l < ESLT_COUNT; ++l)
		{
			ActiveNodeList[l].clear();
		}
	}

	void TScene::AddToActiveList(E_SCENE_LIST_TYPE List, TNode * ActiveNode)
	{
		ActiveNodeList[List].push_back(ActiveNode);
	}

	void TScene::BindLights()
	{
		if (ActiveNodeList[ESLT_LIGHTS].size() > 0)
		{
			bool ForceRebind = HasSceneFlag(SF_LIGHTS_DIRTY);

			// go though static solid list
			for (auto StaticMeshNode : ActiveNodeList[ESLT_STATIC_SOLID])
			{
				TI_ASSERT(StaticMeshNode->GetType() == ENT_StaticMesh);
				StaticMeshNode->BindLights(ActiveNodeList[ESLT_LIGHTS], ForceRebind);
			}

			// then dynamic solid list
			for (auto DynamicMeshNode : ActiveNodeList[ESLT_DYNAMIC_SOLID])
			{
				DynamicMeshNode->BindLights(ActiveNodeList[ESLT_LIGHTS], ForceRebind);
			}
		}
	}
}
