/*
	TiX Engine v2.0 Copyright (C) 2018~2021
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
			// Init Loaded env light array
			NodeSceneTile->LoadedEnvLightInfos.resize(NodeSceneTile->SceneTileResource->EnvLightInfos.size());
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
		TI_TODO("Remove scene tile resources from FScene, like meshes and env lights");
	}

	void TNodeSceneTile::Tick(float Dt)
	{
		TNode::Tick(Dt);

		// check if Asset is loaded
		if (SceneTileResource != nullptr)
		{
			LoadStaticMeshes();
			LoadEnvCubemaps();
		}
	}

	void TNodeSceneTile::LoadStaticMeshes()
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
						for (uint32 Section = 0; Section < TotalSections; ++Section)
						{
							TI_ASSERT(StaticMesh->GetMeshBuffer()->MeshBufferResource != nullptr);

							const TMeshSection& MeshSection = StaticMesh->GetMeshSection(Section);

							FPrimitivePtr Primitive = ti_new FPrimitive;
							Primitive->SetMesh(
								StaticMesh->GetMeshBuffer()->MeshBufferResource,
								MeshSection.IndexStart,
								MeshSection.Triangles,
								MeshSection.DefaultMaterial,
								SceneTileResource->MeshInstanceBuffer->InstanceResource,
								SceneTileResource->InstanceCountAndOffset[MeshSectionOffset + Section].X,
								SceneTileResource->InstanceCountAndOffset[MeshSectionOffset + Section].Y
							);
							Primitive->SetSceneTilePos(SceneTileResource->Position);
							LinkedPrimitives.push_back(Primitive);
							PrimitiveIndices.push_back(MeshSectionOffset + Section);
						}

						// Add static mesh to scene
						FMeshBufferPtr OccludeMeshBufferResource = StaticMesh->GetOccludeMesh() == nullptr ? nullptr : StaticMesh->GetOccludeMesh()->MeshBufferResource;
						FMeshBufferPtr StaticMeshResource = StaticMesh->GetMeshBuffer()->MeshBufferResource;
						FUniformBufferPtr ClusterData = StaticMesh->GetMeshBuffer()->MeshClusterDataResource;
						ENQUEUE_RENDER_COMMAND(AddTSceneTileStaticMeshToFScene)(
							[StaticMeshResource, OccludeMeshBufferResource, ClusterData]()
							{
								FRenderThread::Get()->GetRenderScene()->AddSceneMeshBuffer(StaticMeshResource, OccludeMeshBufferResource, ClusterData);
							});


						// Add primitive to scene
						FSceneTileResourcePtr RenderThreadSceneTileResource = SceneTileResource->RenderThreadTileResource;
						TVector<uint32> Indices = PrimitiveIndices;
						TVector<FPrimitivePtr> Primitives = LinkedPrimitives;
						ENQUEUE_RENDER_COMMAND(AddTSceneTileMeshPrimitivesToFSceneTile)(
							[RenderThreadSceneTileResource, Indices, Primitives]()
							{
								const uint32 TotalPrimitives = (uint32)Primitives.size();
								for (uint32 p = 0; p < TotalPrimitives; ++p)
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

	void TNodeSceneTile::LoadEnvCubemaps()
	{
		const uint32 EnvCubemapCount = (uint32)SceneTileResource->EnvLights.size();
		if (EnvCubemapCount > 0)
		{
			uint32 LoadedCubemapCount = 0;
			for (uint32 c = 0; c < EnvCubemapCount; ++c)
			{
				TAssetPtr EnvLightAsset = SceneTileResource->EnvLights[c];
				const TSceneTileResource::TEnvLightInfo& EnvLightInfo = SceneTileResource->EnvLightInfos[c];
				if (EnvLightAsset != nullptr)
				{
					if (EnvLightAsset->IsLoaded())
					{
						const TVector<TResourcePtr>& CubeResources = EnvLightAsset->GetResources();
						TI_ASSERT(CubeResources[0]->GetType() == ERES_TEXTURE);
						TTexturePtr CubeTexture = static_cast<TTexture*>(CubeResources[0].get());

						// Add Env Light to FScene
						FTexturePtr CubeTextureResource = CubeTexture->TextureResource;
						vector3df Position = EnvLightInfo.Position;
						ENQUEUE_RENDER_COMMAND(AddEnvLightToFScene)(
							[CubeTextureResource, Position]()
							{
								FRenderThread::Get()->GetRenderScene()->AddEnvLight(CubeTextureResource, Position);
							});

						// Remove the reference holder
						TI_ASSERT(LoadedEnvLightInfos[c].Radius == 0.f);
						LoadedEnvLightInfos[c] = EnvLightInfo;
						SceneTileResource->EnvLights[c] = nullptr;

						++LoadedCubemapCount;
					}
				}
				else
				{
					++LoadedCubemapCount;
				}
			}
			TI_ASSERT(LoadedCubemapCount <= EnvCubemapCount);
			if (LoadedCubemapCount == EnvCubemapCount)
			{
				SceneTileResource->EnvLights.clear();
			}
		}
	}
}
