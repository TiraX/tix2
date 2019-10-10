/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TNodeSceneTile.h"

namespace tix
{
	TSceneTileLoadingFinishDelegate::TSceneTileLoadingFinishDelegate(const TString& InLevelName, const TString& InTileName)
		: LevelName(InLevelName)
		, TileName(InTileName)
	{}

	TSceneTileLoadingFinishDelegate::~TSceneTileLoadingFinishDelegate()
	{}

	void TSceneTileLoadingFinishDelegate::LoadingFinished(TAssetPtr InAsset)
	{
		TI_ASSERT(IsGameThread());

		// Try to find Parent Level Node
		TNode* NodeLevel = TEngine::Get()->GetScene()->GetRoot()->GetNodeById(LevelName);
		if (NodeLevel != nullptr)
		{
			TI_ASSERT(NodeLevel->GetType() == ENT_Level);
			// Create scene tile node
			TNodeSceneTile * NodeSceneTile = TNodeFactory::CreateNode<TNodeSceneTile>(NodeLevel, TileName);

			// Hold references of Instances
			const TVector<TResourcePtr>& Resources = InAsset->GetResources();
			TI_ASSERT(Resources.size() == 1);
			TResourcePtr FirstResource = Resources[0];
			TI_ASSERT(FirstResource->GetType() == ERES_SCENE_TILE);

			NodeSceneTile->SceneTileResource = static_cast<TSceneTileResource*>(FirstResource.get());
			TI_ASSERT(NodeSceneTile->SceneTileResource->Meshes.size() == NodeSceneTile->SceneTileResource->InstanceOffsetAndCount.size());

			// Register scene tile info to FSceneMetaInfos, for GPU tile frustum cull
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(AddSceneTileRes,
				TSceneTileResourcePtr, SceneTileRes, NodeSceneTile->SceneTileResource,
				{
					FRenderThread::Get()->GetRenderScene()->AddSceneTileInfo(SceneTileRes);
				});

			TI_TODO("Remove static mesh node creation, send meshes to FScene by NodeSceneTile directly.");
			// Create static mesh nodes , assign mesh and instance res to it.
			const int32 MeshesCount = (int32)NodeSceneTile->SceneTileResource->Meshes.size();
			for (int32 m = 0 ; m < MeshesCount ; ++ m)
			{
				TAssetPtr MeshAsset = NodeSceneTile->SceneTileResource->Meshes[m];
				TInstanceBufferPtr Instance = NodeSceneTile->SceneTileResource->MeshInstanceBuffer;
				const vector2di& InstanceOffsetAndCount = NodeSceneTile->SceneTileResource->InstanceOffsetAndCount[m];

				TNodeStaticMesh * NodeStaticMesh = TNodeFactory::CreateNode<TNodeStaticMesh>(NodeSceneTile, MeshAsset->GetName());
				NodeStaticMesh->LinkMeshAsset(MeshAsset, Instance, InstanceOffsetAndCount.Y, InstanceOffsetAndCount.X, false, false);
			}

			TI_TODO("Hold a scene tile resource temp, hold instances only in futher.");
			TI_TODO("Scene tile resource do not need to hold TVector<TAssetPtr> Meshes, remove it in futher");
		}
		else
		{
			_LOG(Warning, "Failed to find level node - [%s]\n", LevelName.c_str());
		}
	}

	//////////////////////////////////////////////////////////////////////////

	TNodeLevel::TNodeLevel(TNode* parent)
		: TNode(TNodeLevel::NODE_TYPE, parent)
	{
		TI_TODO("Add a THMap for finding tile node more fast.");
		TI_TODO("Override GetNodeByID method.");
	}

	TNodeLevel::~TNodeLevel()
	{
	}

	//////////////////////////////////////////////////////////////////////////

	TNodeSceneTile::TNodeSceneTile(TNode* parent)
		: TNode(TNodeSceneTile::NODE_TYPE, parent)
	{
	}

	TNodeSceneTile::~TNodeSceneTile()
	{
	}
}
