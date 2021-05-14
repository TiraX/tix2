/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	// Hold all resources in a tile, like meshes, instances, etc
	class TSceneTileResource : public TResource
	{
	public:
		TSceneTileResource();
		~TSceneTileResource();

		TInstanceBufferPtr GetInstanceBuffer()
		{
			return SMInstanceBuffer;
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
		// static meshes always processed with instances
		uint32 TotalStaticMeshes;
		TVector<TAssetPtr> StaticMeshes;
		uint32 TotalSMSections;	// Total static mesh sections
		uint32 TotalSMInstances;	// Total static mesh instances
		TVector<vector2di> SMInstanceCountAndOffset;	// X is Count, Y is Offset
		TVector<uint32> SMSectionsCount;
		TInstanceBufferPtr SMInstanceBuffer;

		// Skeletal Meshes
		// skeletal mesh always processed with actors
		uint32 TotalSkeletalMeshes;
		TVector<TAssetPtr> SkeletalMeshes;
		uint32 TotalSKMActors;

		FSceneTileResourcePtr RenderThreadTileResource;
	};
}
