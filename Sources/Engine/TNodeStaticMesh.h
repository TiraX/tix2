/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TNodeStaticMesh : public TNode
	{
		DECLARE_NODE_WITH_CONSTRUCTOR(StaticMesh);

	public:
		virtual ~TNodeStaticMesh();

		void LinkFPrimitive(FPrimitivePtr Primitive);
	protected:
		FPrimitivePtr LinkedPrimitive;
	};

} // end namespace tix

