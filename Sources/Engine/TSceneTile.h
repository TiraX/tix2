/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TSceneTile : public TResource
	{
	public:
		TSceneTile();
		~TSceneTile();

	public:
		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

	protected:

	protected:
		vector2di Position;
		aabbox3df BBox;
		TVector<TAssetPtr> Meshes;
		TVector<TInstanceBufferPtr> MeshInstances;

		friend class TAssetFile;
	};
}
