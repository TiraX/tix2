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

		virtual void BindLights(TVector<TNode*>& Lights, bool ForceRebind) override;
		void LinkMesh(TMeshBufferPtr InMesh, TMaterialInstancePtr InMInstance, bool bCastShadow, bool bReceiveShadow);

	protected:
		virtual void UpdateAbsoluteTransformation() override;

	protected:
		FPrimitivePtr LinkedPrimitive;
		aabbox3df TransformedBBox;
		TVector<TNodeLight*> BindedLights;
	};

} // end namespace tix

