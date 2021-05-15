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
			TI_ASSERT(NodeSceneTile->SceneTileResource->SMInfos.TotalSections == NodeSceneTile->SceneTileResource->SMInstances.InstanceCountAndOffset.size());
			TI_ASSERT(NodeSceneTile->SceneTileResource->SMInfos.NumMeshes == NodeSceneTile->SceneTileResource->SMInfos.SectionsCount.size());
			TI_ASSERT(NodeSceneTile->LoadedStaticMeshAssets.empty() && NodeSceneTile->LoadedSkeletalMeshAssets.empty());
			// Init Loaded mesh asset array.
			NodeSceneTile->LoadedStaticMeshAssets.resize(NodeSceneTile->SceneTileResource->SMInfos.MeshAssets.size());
			NodeSceneTile->LoadedSkeletalMeshAssets.resize(NodeSceneTile->SceneTileResource->SKMInfos.MeshAssets.size());
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
			LoadSkeletalMeshes();
			LoadEnvCubemaps();
		}
	}

	void TNodeSceneTile::LoadStaticMeshes()
	{
		const uint32 StaticMeshCount = (uint32)SceneTileResource->SMInfos.MeshAssets.size();
		if (StaticMeshCount > 0)
		{
			uint32 LoadedMeshCount = 0;
			uint32 MeshSectionOffset = 0;
			for (uint32 m = 0; m < StaticMeshCount; ++m)
			{
				TAssetPtr MeshAsset = SceneTileResource->SMInfos.MeshAssets[m];

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
						TI_ASSERT(TotalSections > 0 && SceneTileResource->SMInfos.SectionsCount[m] == TotalSections);
						LinkedPrimitives.reserve(TotalSections);
						PrimitiveIndices.reserve(TotalSections);
						for (uint32 Section = 0; Section < TotalSections; ++Section)
						{
							TI_ASSERT(StaticMesh->GetMeshBuffer()->MeshBufferResource != nullptr);

							const TMeshSection& MeshSection = StaticMesh->GetMeshSection(Section);

							FPrimitivePtr Primitive = ti_new FPrimitive;
							Primitive->SetInstancedStaticMesh(
								StaticMesh->GetMeshBuffer()->MeshBufferResource,
								MeshSection.IndexStart,
								MeshSection.Triangles,
								MeshSection.DefaultMaterial,
								SceneTileResource->SMInstances.InstanceBuffer->InstanceResource,
								SceneTileResource->SMInstances.InstanceCountAndOffset[MeshSectionOffset + Section].X,
								SceneTileResource->SMInstances.InstanceCountAndOffset[MeshSectionOffset + Section].Y
							);
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
						TI_ASSERT(LoadedStaticMeshAssets[m] == nullptr);
						LoadedStaticMeshAssets[m] = MeshAsset;
						SceneTileResource->SMInfos.MeshAssets[m] = nullptr;

						++LoadedMeshCount;
					}
				}
				else
				{
					++LoadedMeshCount;
				}
				MeshSectionOffset += SceneTileResource->SMInfos.SectionsCount[m];
			}
			TI_ASSERT(LoadedMeshCount <= StaticMeshCount);
			if (LoadedMeshCount == StaticMeshCount)
			{
				SceneTileResource->SMInfos.MeshAssets.clear();
			}
		}
	}

	void TNodeSceneTile::LoadSkeletalMeshes()
	{
		const uint32 SkeletalMeshCount = (uint32)SceneTileResource->SKMInfos.MeshAssets.size();
		if (SkeletalMeshCount > 0)
		{
			uint32 LoadedMeshCount = 0;
			uint32 MeshSectionOffset = 0;
			for (uint32 m = 0; m < SkeletalMeshCount; ++m)
			{
				TAssetPtr MeshAsset = SceneTileResource->SKMInfos.MeshAssets[m];

				if (MeshAsset != nullptr)
				{
					if (MeshAsset->IsLoaded())
					{
						const TVector<TResourcePtr>& MeshResources = MeshAsset->GetResources();
						TI_ASSERT(MeshResources[0]->GetType() == ERES_STATIC_MESH);
						TStaticMeshPtr StaticMesh = static_cast<TStaticMesh*>(MeshResources[0].get());

						// Go through all actors used this mesh
						for (uint32 a = 0; a < SceneTileResource->SKMActorInfos.size(); a++)
						{
							const TSkeletalMeshActorInfo& SKMActorInfo = SceneTileResource->SKMActorInfos[a];
							if (SKMActorInfo.MeshAssetRef == MeshAsset)
							{
								// Skeleton asset
								TI_ASSERT(SKMActorInfo.SkeletonAsset != nullptr && SKMActorInfo.SkeletonAsset->IsLoaded());
								const TVector<TResourcePtr>& SkeletonResources = SKMActorInfo.SkeletonAsset->GetResources();
								TI_ASSERT(SkeletonResources[0]->GetType() == ERES_SKELETON);
								TSkeletonPtr Skeleton = static_cast<TSkeleton*>(SkeletonResources[0].get());

								// Animation asset
								TAnimSequencePtr Anim = nullptr;
								if (SKMActorInfo.AnimAsset != nullptr)
								{
									const TVector<TResourcePtr>& AnimResources = SKMActorInfo.AnimAsset->GetResources();
									TI_ASSERT(AnimResources[0]->GetType() == ERES_ANIM_SEQUENCE);
									Anim = static_cast<TAnimSequence*>(AnimResources[0].get());
								}

								// Create skeletal node
								TNodeSkeletalMesh* NodeSkeletalMesh = TNodeFactory::CreateNode<TNodeSkeletalMesh>(this, MeshAsset->GetName());
								NodeSkeletalMesh->SetSceneTileResource(SceneTileResource);
								NodeSkeletalMesh->LinkMeshAndSkeleton(StaticMesh, Skeleton);
								NodeSkeletalMesh->SetAnimation(Anim);
							}
						}

						// Remove the reference holder
						TI_ASSERT(LoadedSkeletalMeshAssets[m] == nullptr);
						LoadedSkeletalMeshAssets[m] = MeshAsset;
						SceneTileResource->SKMInfos.MeshAssets[m] = nullptr;

						++LoadedMeshCount;
					}
				}
				else
				{
					++LoadedMeshCount;
				}
				MeshSectionOffset += SceneTileResource->SMInfos.SectionsCount[m];
			}
			TI_ASSERT(LoadedMeshCount <= SkeletalMeshCount);
			if (LoadedMeshCount == SkeletalMeshCount)
			{
				SceneTileResource->SMInfos.MeshAssets.clear();
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
