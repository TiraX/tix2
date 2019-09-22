/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TStaticMeshLoadingFinishDelegate : public ILoadingFinishDelegate
	{
	public:
		TStaticMeshLoadingFinishDelegate(const TString& InLevelName, const vector2di& InSceneTilePos, int32 InMeshIndex);
		virtual ~TStaticMeshLoadingFinishDelegate();

		virtual void LoadingFinished(TAssetPtr InAsset) override;

	private:
		TString LevelName;
		vector2di SceneTilePos;
		int32 MeshIndexInTile;
	};

	//////////////////////////////////////////////////////////////////////////

	class TNodeLight;
	class TNodeStaticMesh : public TNode
	{
		DECLARE_NODE_WITH_CONSTRUCTOR(StaticMesh);

	public:
		virtual ~TNodeStaticMesh();

		virtual void BindLights(TVector<TNode*>& Lights, bool ForceRebind) override;
		void LinkMesh(const TVector<TMeshBufferPtr>& InMeshes, TInstanceBufferPtr InInstanceBuffer, bool bCastShadow, bool bReceiveShadow);

	protected:
		virtual void UpdateAbsoluteTransformation() override;

	protected:
		TVector<FPrimitivePtr> LinkedPrimitives;
		aabbox3df TransformedBBox;
		TVector<TNodeLight*> BindedLights;
	};

} // end namespace tix

