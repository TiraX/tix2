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

		// Create Primitive
		FPrimitivePtr Primitive = ti_new FPrimitive;
		TMaterialPtr Material = InMInstance->LinkedMaterial;
		TI_ASSERT((InMesh->MeshBufferResource != nullptr) 
			&& (Material->Pipeline->PipelineResource != nullptr)
			&& (InMInstance->UniformBuffer != nullptr));
		Primitive->AddMesh(InMesh->MeshBufferResource, Material->Pipeline->PipelineResource, InMInstance);

		// Add primitive to scene
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(AddPrimitveToScene,
			FPrimitivePtr, Primitive, Primitive,
			{
				RenderThread->GetRenderScene()->AddPrimitive(Primitive);
			});

		// Link primitive to node
		StaticMesh->LinkFPrimitive(Primitive);
		
		return StaticMesh;
	}
}
