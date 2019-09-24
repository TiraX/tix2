/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TNodeLight;
	class TNodeStaticMesh : public TNode
	{
		DECLARE_NODE_WITH_CONSTRUCTOR(StaticMesh);

	public:
		virtual ~TNodeStaticMesh();

		virtual void UpdateAllTransformation() override;

		virtual void BindLights(TVector<TNode*>& Lights, bool ForceRebind) override;
		void LinkMeshAsset(TAssetPtr InMeshAsset, TInstanceBufferPtr InInstanceBuffer, bool bCastShadow, bool bReceiveShadow);
		void LinkMeshBuffer(const TVector<TMeshBufferPtr>& InMeshes, TInstanceBufferPtr InInstanceBuffer, bool bCastShadow, bool bReceiveShadow);

	protected:
		virtual void UpdateAbsoluteTransformation() override;

	protected:
		TVector<FPrimitivePtr> LinkedPrimitives;
		aabbox3df TransformedBBox;
		TVector<TNodeLight*> BindedLights;

		TAssetPtr MeshAsset;
		TInstanceBufferPtr MeshInstance;
	};

} // end namespace tix

