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
			TI_ASSERT(NodeSceneTile->SceneTileResource->Meshes.size() == NodeSceneTile->SceneTileResource->InstanceCountAndOffset.size());
			TI_ASSERT(NodeSceneTile->LoadedMeshAssets.empty());
			// Init Loaded mesh asset array.
			NodeSceneTile->LoadedMeshAssets.resize(NodeSceneTile->SceneTileResource->Meshes.size());

			// Register scene tile info to FSceneMetaInfos, for GPU tile frustum cull
			FSceneTileResourcePtr RenderThreadTileResource = ti_new FSceneTileResource(NodeSceneTile->SceneTileResource);
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(AddSceneTileRes,
				FSceneTileResourcePtr, SceneTileRes, RenderThreadTileResource,
				{
					FRenderThread::Get()->GetRenderScene()->AddSceneTileInfo(SceneTileRes);
				});
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

	void TNodeSceneTile::UpdateAllTransformation()
	{
		TNode::UpdateAllTransformation();

		// check if Asset is loaded
		if (SceneTileResource != nullptr)
		{
			const uint32 MeshCount = (uint32)SceneTileResource->Meshes.size();
			if (MeshCount > 0)
			{
				uint32 LoadedMeshCount = 0;
				for (uint32 m = 0; m < MeshCount; ++m)
				{
					TAssetPtr MeshAsset = SceneTileResource->Meshes[m];

					if (MeshAsset != nullptr)
					{
						if (MeshAsset->IsLoaded())
						{
							// Gather loaded mesh resources
							TVector<FPrimitivePtr> LinkedPrimitives;
							LinkedPrimitives.reserve(MeshAsset->GetResources().size());
							for (auto Res : MeshAsset->GetResources())
							{
								TMeshBufferPtr Mesh = static_cast<TMeshBuffer*>(Res.get());
								TI_ASSERT(Mesh->MeshBufferResource != nullptr);

								FPrimitivePtr Primitive = ti_new FPrimitive;
								Primitive->SetMesh(
									Mesh->MeshBufferResource,
									Mesh->GetBBox(),
									Mesh->GetDefaultMaterial(),
									SceneTileResource->MeshInstanceBuffer->InstanceResource,
									SceneTileResource->InstanceCountAndOffset[m].X,
									SceneTileResource->InstanceCountAndOffset[m].Y
								);
								Primitive->SetIndexInSceneTile(m);
								Primitive->SetSceneTilePos(SceneTileResource->Position);
								LinkedPrimitives.push_back(Primitive);
							}

							// Add primitive to scene
							ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(AddSceneTileMeshPrimitivesToScene,
								TVector<FPrimitivePtr>, Primitives, LinkedPrimitives,
								{
									FRenderThread::Get()->GetRenderScene()->AddStaticMeshPrimitives(Primitives);
								});

							// Remove the reference holder
							TI_ASSERT(LoadedMeshAssets[m] == nullptr);
							LoadedMeshAssets[m] = MeshAsset;
							SceneTileResource->Meshes[m] = nullptr;

							++LoadedMeshCount;
						}
					}
					else
					{
						++LoadedMeshCount;
					}
				}
				TI_ASSERT(LoadedMeshCount <= MeshCount);
				if (LoadedMeshCount == MeshCount)
				{
					SceneTileResource->Meshes.clear();
				}
			}
		}
	}
}
