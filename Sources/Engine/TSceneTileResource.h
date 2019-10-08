/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
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
		TVector<TAssetPtr> Meshes;
		// X is Offset, Y is Count
		TVector<vector2di> InstanceOffsetAndCount;
		TInstanceBufferPtr MeshInstanceBuffer;
	};
}
