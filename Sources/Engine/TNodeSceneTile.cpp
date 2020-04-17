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
			TI_ASSERT(NodeSceneTile->SceneTileResource->TotalMeshSections == NodeSceneTile->SceneTileResource->InstanceCountAndOffset.size());
			TI_ASSERT(NodeSceneTile->SceneTileResource->TotalMeshes == NodeSceneTile->SceneTileResource->MeshSectionsCount.size());
			TI_ASSERT(NodeSceneTile->LoadedMeshAssets.empty());
			// Init Loaded mesh asset array.
			NodeSceneTile->LoadedMeshAssets.resize(NodeSceneTile->SceneTileResource->Meshes.size());
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
				uint32 MeshSectionOffset = 0;
				for (uint32 m = 0; m < MeshCount; ++m)
				{
					TAssetPtr MeshAsset = SceneTileResource->Meshes[m];

					if (MeshAsset != nullptr)
					{
						if (MeshAsset->IsLoaded())
						{
							// Gather loaded mesh resources
							TVector<FPrimitivePtr> LinkedPrimitives;
							TVector<uint32> PrimitiveIndices;

							const TVector<TResourcePtr>& MeshResources = MeshAsset->GetResources();
							TI_ASSERT(MeshResources[0]->GetType() == ERES_STATIC_MESH);
							TStaticMeshPtr StaticMesh = static_cast<TStaticMesh*>(MeshResources[0].get());
							// MeshResources Include mesh sections and 1 collision set
							// Mesh sections
							const uint32 TotalSections = StaticMesh->GetMeshSectionCount();
							TI_ASSERT(TotalSections > 0 && SceneTileResource->MeshSectionsCount[m] == TotalSections);
							LinkedPrimitives.reserve(TotalSections);
							PrimitiveIndices.reserve(TotalSections);
							for (uint32 Section = 0 ; Section < TotalSections ; ++ Section)
							{
								TI_ASSERT(StaticMesh->GetMeshBuffer()->MeshBufferResource != nullptr);

								const TMeshSection& MeshSection = StaticMesh->GetMeshSection(Section);

								FPrimitivePtr Primitive = ti_new FPrimitive;
								Primitive->SetMesh(
									StaticMesh->GetMeshBuffer()->MeshBufferResource,
									MeshSection.IndexStart,
									MeshSection.Triangles,
									StaticMesh->GetMeshBuffer()->GetBBox(),
									MeshSection.DefaultMaterial,
									SceneTileResource->MeshInstanceBuffer->InstanceResource,
									SceneTileResource->InstanceCountAndOffset[MeshSectionOffset + Section].X,
									SceneTileResource->InstanceCountAndOffset[MeshSectionOffset + Section].Y
								);
								Primitive->SetClusterMetaData(StaticMesh->GetMeshBuffer()->MeshClusterDataResource);
								Primitive->SetSceneTilePos(SceneTileResource->Position);
								LinkedPrimitives.push_back(Primitive);
								PrimitiveIndices.push_back(MeshSectionOffset + Section);
							}

							// Collisions
							if (StaticMesh->GetOccludeMesh() != nullptr)
							{
								TI_ASSERT(StaticMesh->GetOccludeMesh()->MeshBufferResource != nullptr);
								LinkedPrimitives[0]->SetOccluder(StaticMesh->GetOccludeMesh()->MeshBufferResource);
							}
							TI_TODO("REFACTOR static mesh, A mesh should include primitives, collisions, instances, occluders");

							// Add primitive to scene
							ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(AddTSceneTileMeshPrimitivesToFSceneTile,
								FSceneTileResourcePtr, RenderThreadSceneTileResource, SceneTileResource->RenderThreadTileResource, 
								TVector<uint32>, Indices, PrimitiveIndices,
								TVector<FPrimitivePtr>, Primitives, LinkedPrimitives,
								{
									const uint32 TotalPrimitives = (uint32)Primitives.size();
									for (uint32 p = 0 ; p < TotalPrimitives ; ++ p)
									{
										RenderThreadSceneTileResource->AddPrimitive(Indices[p], Primitives[p]);
									}
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
					MeshSectionOffset += SceneTileResource->MeshSectionsCount[m];
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
