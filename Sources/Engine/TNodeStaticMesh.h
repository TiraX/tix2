/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TNodeLight;
	class TNodeStaticMesh : public TNode, public ILoadingTaskNotifier
	{
		DECLARE_NODE_WITH_CONSTRUCTOR(StaticMesh);

	public:
		virtual ~TNodeStaticMesh();

		virtual void BindLights(TVector<TNode*>& Lights, bool ForceRebind) override;
		void LinkMesh(const TVector<TMeshBufferPtr>& InMeshes, TInstanceBufferPtr InInstanceBuffer, bool bCastShadow, bool bReceiveShadow);

		void SetMeshIndexInTile(int32 Index)
		{
			MeshIndexInTile = Index;
		}

		// interface from ILoadingTaskNotifier
		virtual void NotifyLoadingFinished(TAssetPtr InAsset) override;

	protected:
		virtual void UpdateAbsoluteTransformation() override;

	protected:
		TVector<FPrimitivePtr> LinkedPrimitives;
		aabbox3df TransformedBBox;
		TVector<TNodeLight*> BindedLights;

		// Remember the mesh index in scene tile in order to find instance buffer stored in scene tile node.
		int32 MeshIndexInTile;
	};

} // end namespace tix

