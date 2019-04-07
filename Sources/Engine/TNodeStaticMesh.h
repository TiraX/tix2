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
		void LinkMesh(TMeshBufferPtr InMesh, TMaterialInstancePtr InMInstance, bool bCastShadow, bool bReceiveShadow);

		// interface from ILoadingTaskNotifier
		virtual void NotifyLoadingFinished(void * Context) override;

	protected:
		virtual void UpdateAbsoluteTransformation() override;

	protected:
		TVector<FPrimitivePtr> LinkedPrimitives;
		aabbox3df TransformedBBox;
		TVector<TNodeLight*> BindedLights;
	};

} // end namespace tix

