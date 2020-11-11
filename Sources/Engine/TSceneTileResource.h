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
			return MeshInstanceBuffer;
		}

	public:
		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

	public:
		TString LevelName;
		vector2di Position;
		aabbox3df BBox;

		uint32 TotalReflectionCaptures;
		TVector<TAssetPtr> EnvCubemaps;
		struct TEnvCubes
		{
			vector3df Position;
			float Radius;
		};
		TVector<TEnvCubes> EnvCubemapInfos;

		uint32 TotalMeshes;
		uint32 TotalMeshSections;
		uint32 TotalInstances;
		TVector<TAssetPtr> Meshes;
		// X is Count, Y is Offset
		TVector<vector2di> InstanceCountAndOffset;
		TVector<uint32> MeshSectionsCount;
		TInstanceBufferPtr MeshInstanceBuffer;

		FSceneTileResourcePtr RenderThreadTileResource;
	};
}
