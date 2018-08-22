/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TNodeStaticMesh.h"

namespace tix
{
	TNodeStaticMesh::TNodeStaticMesh(TNode* parent)
		: TNode(TNodeStaticMesh::NODE_TYPE, parent)
	{
	}

	TNodeStaticMesh::~TNodeStaticMesh()
	{
		// Remove Primitive from scene
		if (LinkedPrimitive != nullptr)
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(RemovePrimitiveFromScene,
				FPrimitivePtr, Primitive, LinkedPrimitive,
				{
					RenderThread->GetRenderScene()->RemovePrimitive(Primitive);
				});
			LinkedPrimitive = nullptr;
		}
	}

	void TNodeStaticMesh::LinkFPrimitive(FPrimitivePtr Primitive)
	{
		LinkedPrimitive = Primitive;
	}
}
