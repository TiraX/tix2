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

	void TNodeStaticMesh::LinkMesh(TMeshBufferPtr InMesh, TMaterialInstancePtr InMInstance, bool bCastShadow, bool bReceiveShadow)
	{
		// Create Primitive
		FPrimitivePtr Primitive = ti_new FPrimitive;
		TI_ASSERT((InMesh->MeshBufferResource != nullptr)
			&& (InMInstance->UniformBuffer != nullptr));
		Primitive->AddMesh(InMesh->MeshBufferResource, InMesh->GetBBox(), InMInstance);

		// Add primitive to scene
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(AddPrimitveToScene,
			FPrimitivePtr, Primitive, Primitive,
			{
				RenderThread->GetRenderScene()->AddPrimitive(Primitive);
			});


		LinkedPrimitive = Primitive;
	}

	void TNodeStaticMesh::UpdateAbsoluteTransformation()
	{
		TNode::UpdateAbsoluteTransformation();

		if (HasFlag(ENF_ABSOLUTETRANSFORMATION_UPDATED))
		{
			TI_ASSERT(LinkedPrimitive  != nullptr);
			TransformedBBox = LinkedPrimitive->BBox;
			AbsoluteTransformation.transformBoxEx(TransformedBBox);
		}

		// add this to static solid list
		TEngine::Get()->GetScene()->AddToActiveList(ESLT_STATIC_SOLID, this);
	}

	void TNodeStaticMesh::BindLights(TVector<TNode *>& Lights, bool ForceRebind)
	{
		if (HasFlag(ENF_ABSOLUTETRANSFORMATION_UPDATED) || ForceRebind)
		{
			BindedLights.clear();

			// find out lights affect this mesh
			for (auto Light : Lights)
			{
				TI_ASSERT(Light->GetType() == ENT_Light);
				TNodeLight * LightNode = static_cast<TNodeLight*>(Light);
				if (TransformedBBox.intersectsWithBox(LightNode->GetAffectBox()))
				{
					BindedLights.push_back(LightNode);
				}
			}

			// create lightinfo uniform buffers
		}
	}
}
