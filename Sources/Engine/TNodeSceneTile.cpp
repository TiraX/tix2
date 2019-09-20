/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TNodeSceneTile.h"

namespace tix
{
	TNodeLevel::TNodeLevel(TNode* parent)
		: TNode(TNodeLevel::NODE_TYPE, parent)
	{
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

	void TNodeSceneTile::NotifyLoadingFinished(TAssetPtr InAsset)
	{
		TI_ASSERT(IsGameThread());

		// Hold references of Instances

		const TVector<TResourcePtr>& Resources = InAsset->GetResources();
		TI_ASSERT(Resources.size() == 1);
		TResourcePtr FirstResource = Resources[0];
		TI_ASSERT(FirstResource->GetType() == ERES_SCENE_TILE);

		SceneTileResource = static_cast<TSceneTileResource*>(FirstResource.get());
		TI_ASSERT(SceneTileResource->Meshes.size() == SceneTileResource->MeshInstances.size());

		TI_TODO("Hold a scene tile resource temp, hold instances only in futher.");
		TI_TODO("Scene tile resource do not need to hold TVector<TAssetPtr> Meshes, remove it in futher");

		// Mesh asset load finished add it to TScene
//		// Create static mesh node
//		const int32 MeshCount = (int32)SceneTileResource->Meshes.size();
//		for (int32 m = 0 ; m < MeshCount ; ++ m)
//		{
//			TNodeStaticMesh * NodeStaticMesh = TNodeFactory::CreateNode<TNodeStaticMesh>(this);
//
//			TAssetPtr MeshRes = SceneTileResource->Meshes[m];
//			TInstanceBufferPtr InstanceBuffer = SceneTileResource->MeshInstances[m];
//
//			// Gather mesh resources
//			TVector<TMeshBufferPtr> Meshes;
//			Meshes.reserve(MeshRes->GetResources().size());
//			for (auto Res : MeshRes->GetResources())
//			{
//				TMeshBufferPtr Mesh = static_cast<TMeshBuffer*>(Res.get());
//				Meshes.push_back(Mesh);
//			}
//			NodeStaticMesh->LinkMesh(Meshes, InstanceBuffer, false, false);
//		}
//#if (TIX_DEBUG_AYNC_LOADING)
//		_LOG(Log, "SceneTile Notify ~ %s.\n", InAsset->GetName().c_str());
//#endif
	}
}
