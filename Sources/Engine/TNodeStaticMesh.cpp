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
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(RemovePrimitiveFromScene,
				TVector<FPrimitivePtr>, Primitives, LinkedPrimitives,
				{
					FRenderThread::Get()->GetRenderScene()->RemovePrimitives(Primitives);
				});
			LinkedPrimitives.clear();
		}
	}

	void TNodeStaticMesh::LinkMesh(const TVector<TMeshBufferPtr>& InMeshes, TInstanceBufferPtr InInstanceBuffer, bool bCastShadow, bool bReceiveShadow)
	{
		// Create Primitives
		LinkedPrimitives.empty();
		LinkedPrimitives.reserve(InMeshes.size());
		for (const auto& M : InMeshes)
		{
			TI_ASSERT(M->MeshBufferResource != nullptr);
			FPrimitivePtr Primitive = ti_new FPrimitive;
			Primitive->SetMesh(M->MeshBufferResource, M->GetBBox(), M->GetDefaultMaterial(), InInstanceBuffer->InstanceResource);
			LinkedPrimitives.push_back(Primitive);
		}

		// Add primitive to scene
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(AddPrimitivesToScene,
			TVector<FPrimitivePtr>, Primitives, LinkedPrimitives,
			{
				FRenderThread::Get()->GetRenderScene()->AddPrimitives(Primitives);
			});
	}

	void TNodeStaticMesh::UpdateAbsoluteTransformation()
	{
		TNode::UpdateAbsoluteTransformation();

		if (HasFlag(ENF_ABSOLUTETRANSFORMATION_UPDATED))
		{
			TI_ASSERT(LinkedPrimitives.size() > 0);
			TransformedBBox = LinkedPrimitives[0]->GetBBox();
			for (int32 i = 1; i < (int32)LinkedPrimitives.size(); ++i)
			{
				TransformedBBox.addInternalBox(LinkedPrimitives[i]->GetBBox());
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

		Primitive->SetPrimitiveUniform(Binding);
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

		TAssetPtr MeshRes = nullptr;
		// Mesh asset load finished add it to TScene
		TAssetPtr InAsset = static_cast<TAsset*>(Context);
		const TVector<TResourcePtr>& Resources = InAsset->GetResources();
		TI_ASSERT(Resources.size() > 0);
		TResourcePtr FirstResource = Resources[0];
		TInstanceBufferPtr InstanceBuffer = nullptr;
		if (FirstResource->GetType() == ERES_INSTANCE)
		{
			TI_ASSERT(Resources.size() == 1);
			InstanceBuffer = static_cast<TInstanceBuffer*>(FirstResource.get());
			TI_ASSERT(MeshAsset != nullptr && MeshAsset->GetResources().size() > 0);
			MeshRes = MeshAsset;
		}
		else
		{
			TI_ASSERT(FirstResource->GetType() == ERES_MESH);
			MeshRes = InAsset;
		}

		// Gather mesh resources
		TVector<TMeshBufferPtr> Meshes;
		Meshes.reserve(MeshRes->GetResources().size());
		for (auto Res : MeshRes->GetResources())
		{
			TMeshBufferPtr Mesh = static_cast<TMeshBuffer*>(Res.get());
			Meshes.push_back(Mesh);
		}
		LinkMesh(Meshes, InstanceBuffer, false, false);

		// Add to scene root
		TEngine::Get()->GetScene()->AddStaticMeshNode(this);
		_LOG(Log, "Notify ~ %s.\n", InAsset->GetName().c_str());
	}
}
