/*
	TiX Engine v2.0 Copyright (C) 2018~2019
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
		if (LinkedPrimitives.size() > 0)
		{
			TI_TODO("Remove in one RenderCommand.");
			for (auto P : LinkedPrimitives)
			{
				ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(RemovePrimitiveFromScene,
					FPrimitivePtr, Primitive, P,
					{
						FRenderThread::Get()->GetRenderScene()->RemovePrimitive(Primitive);
					});
			}
			LinkedPrimitives.clear();
		}
	}

	void TNodeStaticMesh::LinkMesh(TMeshBufferPtr InMesh, TMaterialInstancePtr InMInstance, bool bCastShadow, bool bReceiveShadow)
	{
		// Create Primitive
		FPrimitivePtr Primitive = ti_new FPrimitive;
		TI_ASSERT(InMesh->MeshBufferResource != nullptr);
		Primitive->AddMesh(InMesh->MeshBufferResource, InMesh->GetBBox(), InMInstance);

		// Add primitive to scene
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(AddPrimitveToScene,
			FPrimitivePtr, Primitive, Primitive,
			{
				FRenderThread::Get()->GetRenderScene()->AddPrimitive(Primitive);
			});


		LinkedPrimitives.push_back(Primitive);
	}

	void TNodeStaticMesh::UpdateAbsoluteTransformation()
	{
		TNode::UpdateAbsoluteTransformation();

		if (HasFlag(ENF_ABSOLUTETRANSFORMATION_UPDATED))
		{
			TI_ASSERT(LinkedPrimitives.size() > 0);
			TransformedBBox = LinkedPrimitives[0]->BBox;
			for (int32 i = 1; i < (int32)LinkedPrimitives.size(); ++i)
			{
				TransformedBBox.addInternalBox(LinkedPrimitives[i]->BBox);
			}
			AbsoluteTransformation.transformBoxEx(TransformedBBox);
		}

		// add this to static solid list
		TEngine::Get()->GetScene()->AddToActiveList(ESLT_STATIC_SOLID, this);
	}

	static void BindLightResource_RenderThread(FPrimitivePtr Primitive, const TVector<FLightPtr>& BindedLightResources)
	{
		FPrimitiveUniformBufferPtr Binding = ti_new FPrimitiveUniformBuffer;
		Binding->UniformBufferData[0].LightsNum.X = (int32)BindedLightResources.size();

		TI_ASSERT(BindedLightResources.size() <= 4);
		for (int32 l = 0; l < (int32)BindedLightResources.size(); ++l)
		{
			FLightPtr LightResource = BindedLightResources[l];
			Binding->UniformBufferData[0].LightIndices[l] = LightResource->GetLightIndex();
		}
		Binding->InitUniformBuffer();

		Primitive->PrimitiveUniformBuffer = Binding;
	}

	void TNodeStaticMesh::BindLights(TVector<TNode *>& Lights, bool ForceRebind)
	{
		TI_TODO("Chnage this to primitive buffer");
		if (HasFlag(ENF_ABSOLUTETRANSFORMATION_UPDATED) || ForceRebind)
		{
			BindedLights.clear();

			// Find out lights affect this mesh
			for (auto Light : Lights)
			{
				TI_ASSERT(Light->GetType() == ENT_Light);
				TNodeLight * LightNode = static_cast<TNodeLight*>(Light);
				if (TransformedBBox.intersectsWithBox(LightNode->GetAffectBox()))
				{
					BindedLights.push_back(LightNode);
				}
			}

			// Create lights info uniform buffers
			TVector<FLightPtr> BindedLightResources;
			if (BindedLights.size() > 0)
			{
				TI_ASSERT(BindedLights.size() <= 4);
				for (int32 l = 0 ; l < (int32)BindedLights.size(); ++ l)
				{
					TNodeLight * Light = BindedLights[l];
					BindedLightResources.push_back(Light->LightResource);
				}
			}
			// Init uniform buffer resource in render thread
			TI_TODO("Share the same uniform buffer for sections in this static mesh.");
			for (auto P : LinkedPrimitives)
			{
				ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(PrimitiveInitLightBindingUB,
					FPrimitivePtr, Primitive, P,
					TVector<FLightPtr>, BindedLightResources, BindedLightResources,
					{
						BindLightResource_RenderThread(Primitive, BindedLightResources);
					});
			}
		}
	}

	void TNodeStaticMesh::NotifyLoadingFinished(void * Context)
	{
		TI_ASSERT(IsGameThread());
		// Mesh asset load finished add it to TScene
		TAssetPtr Asset = static_cast<TAsset*>(Context);
		const TVector<TResourcePtr>& Resources = Asset->GetResources();
		for (auto Res : Resources)
		{
			TI_ASSERT(Res->GetType() == ERES_MESH);
			TMeshBufferPtr MB = static_cast<TMeshBuffer*>(Res.get());
			LinkMesh(MB, MB->GetDefaultMaterial(), false, false);
		}

		// Add to scene root
		TEngine::Get()->GetScene()->AddStaticMeshNode(this);
	}
}
