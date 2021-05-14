/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TMeshInfoInTile
	{
		uint32 NumMeshes;
		uint32 TotalSections;
		TVector<TAssetPtr> MeshAssets;
		TVector<uint32> SectionsCount;

		TMeshInfoInTile()
			: NumMeshes(0)
			, TotalSections(0)
		{}
	};
	struct TStaticMeshInstanceInfo
	{
		uint32 NumInstances;	// Total static mesh instances
		TVector<vector2di> InstanceCountAndOffset;	// X is Count, Y is Offset
		TInstanceBufferPtr InstanceBuffer;

		TStaticMeshInstanceInfo()
			: NumInstances(0)
		{}
	};
	struct TSkeletalMeshActorInfo
	{
		TAssetPtr MeshAssetRef;
		TAssetPtr SkeletonAsset;
		TAssetPtr AnimAsset;
		vector3df Pos;
		quaternion Rot;
		vector3df Scale;
	};

	// Hold all resources in a tile, like meshes, instances, etc
	class TSceneTileResource : public TResource
	{
	public:
		TSceneTileResource();
		~TSceneTileResource();

		TInstanceBufferPtr GetInstanceBuffer()
		{
			return SMInstances.InstanceBuffer;
		}

	public:
		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

	public:
		TString LevelName;
		vector2di Position;
		aabbox3df BBox;

		uint32 TotalEnvLights;
		TVector<TAssetPtr> EnvLights;
		struct TEnvLightInfo
		{
			vector3df Position;
			float Radius;

			TEnvLightInfo()
				: Radius(0.f)
			{}
		};
		TVector<TEnvLightInfo> EnvLightInfos;

		// Static Meshes
		// Static meshes always processed with instances
		TMeshInfoInTile SMInfos;
		TStaticMeshInstanceInfo SMInstances;

		// Skeletal Meshes
		// Skeletal mesh always processed with actors
		TMeshInfoInTile SKMInfos;
		TVector<TSkeletalMeshActorInfo> SKMActorInfos;

		FSceneTileResourcePtr RenderThreadTileResource;
	};
}
