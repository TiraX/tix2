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
		, InstanceCount(0)
		, InstanceOffset(0)
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

	void TNodeStaticMesh::UpdateAllTransformation()
	{
		// check if Asset is loaded
		if (MeshAsset != nullptr)
		{
			if (MeshAsset->IsLoaded())
			{
				// Gather loaded mesh resources
				TVector<TMeshBufferPtr> Meshes;
				Meshes.reserve(MeshAsset->GetResources().size());
				for (auto Res : MeshAsset->GetResources())
				{
					TMeshBufferPtr Mesh = static_cast<TMeshBuffer*>(Res.get());
					Meshes.push_back(Mesh);
				}

				// Create static mesh node
				LinkMeshBuffer(Meshes, MeshInstance, InstanceCount, InstanceOffset, false, false);

				// Remove the reference holder
				MeshAsset = nullptr;
				MeshInstance = nullptr;
			}
		}

		// No asset wait for loading
		if (MeshAsset == nullptr)
		{
			TNode::UpdateAllTransformation();
		}
	}

	void TNodeStaticMesh::LinkMeshBuffer(
		const TVector<TMeshBufferPtr>& InMeshes, 
		TInstanceBufferPtr InInstanceBuffer,
		uint32 InInstanceCount,
		uint32 InInstanceOffset, 
		bool bCastShadow, 
		bool bReceiveShadow)
	{
		// Create Primitives
		LinkedPrimitives.empty();
		LinkedPrimitives.reserve(InMeshes.size());

		TNode* SceneTileParent = GetParent(ENT_SceneTile);
		TNodeSceneTile * SceneTileNode = static_cast<TNodeSceneTile *>(SceneTileParent);
		for (const auto& M : InMeshes)
		{
			TI_ASSERT(M->MeshBufferResource != nullptr);
			FPrimitivePtr Primitive = ti_new FPrimitive;
			Primitive->SetMesh(
				M->MeshBufferResource, 
				M->GetBBox(), 
				M->GetDefaultMaterial(), 
				InInstanceBuffer != nullptr ? InInstanceBuffer->InstanceResource : nullptr,
				InInstanceCount,
				InInstanceOffset
			);
			Primitive->SetParentTilePosition(SceneTileNode->GetTilePosition());
			LinkedPrimitives.push_back(Primitive);
		}

		// Add primitive to scene
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(AddPrimitivesToScene,
			TVector<FPrimitivePtr>, Primitives, LinkedPrimitives,
			{
				FRenderThread::Get()->GetRenderScene()->AddPrimitives(Primitives);
			});
	}

	void TNodeStaticMesh::LinkMeshAsset(
		TAssetPtr InMeshAsset, 
		TInstanceBufferPtr InInstanceBuffer, 
		uint32 InInstanceCount, 
		uint32 InInstanceOffset, 
		bool bCastShadow, 
		bool bReceiveShadow)
	{
		// Save mesh asset
		MeshAsset = InMeshAsset;
		MeshInstance = InInstanceBuffer;
		InstanceCount = InInstanceCount;
		InstanceOffset = InInstanceOffset;
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

			// Init uniform buffer resource in render thread
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(UpdatePrimitiveBuffer,
				TVector<FPrimitivePtr>, Primitives, LinkedPrimitives,
				matrix4, LocalToWorld, AbsoluteTransformation,
				{
					for (auto P : Primitives)
					{
						P->SetLocalToWorld(LocalToWorld);
					}
				});
		}

		// add this to static solid list
		TEngine::Get()->GetScene()->AddToActiveList(ESLT_STATIC_SOLID, this);
	}

	//static void BindLightResource_RenderThread(const TVector<FPrimitivePtr>& Primitives, const TVector<FLightPtr>& BindedLightResources)
	//{
	//	FPrimitiveUniformBufferPtr Binding = ti_new FPrimitiveUniformBuffer;
	//	Binding->UniformBufferData[0].LightsNum.X = (int32)BindedLightResources.size();

	//	TI_ASSERT(BindedLightResources.size() <= 4);
	//	for (int32 l = 0; l < (int32)BindedLightResources.size(); ++l)
	//	{
	//		FLightPtr LightResource = BindedLightResources[l];
	//		Binding->UniformBufferData[0].LightIndices[l] = LightResource->GetLightIndex();
	//	}
	//	Binding->InitUniformBuffer();

	//	for (auto P : Primitives)
	//	{
	//		P->SetPrimitiveUniform(Binding);
	//	}
	//}

	void TNodeStaticMesh::BindLights(TVector<TNode *>& Lights, bool ForceRebind)
	{
		// Dynamic lighting will be re-designed
		TI_ASSERT(0);
		//if (HasFlag(ENF_ABSOLUTETRANSFORMATION_UPDATED) || ForceRebind)
		//{
		//	BindedLights.clear();

		//	// Find out lights affect this mesh
		//	for (auto Light : Lights)
		//	{
		//		TI_ASSERT(Light->GetType() == ENT_Light);
		//		TNodeLight * LightNode = static_cast<TNodeLight*>(Light);
		//		if (TransformedBBox.intersectsWithBox(LightNode->GetAffectBox()))
		//		{
		//			BindedLights.push_back(LightNode);
		//		}
		//	}

		//	// Create lights info uniform buffers
		//	TVector<FLightPtr> BindedLightResources;
		//	if (BindedLights.size() > 0)
		//	{
		//		TI_ASSERT(BindedLights.size() <= 4);
		//		for (int32 l = 0 ; l < (int32)BindedLights.size(); ++ l)
		//		{
		//			TNodeLight * Light = BindedLights[l];
		//			BindedLightResources.push_back(Light->LightResource);
		//		}
		//	}
		//	// Init uniform buffer resource in render thread
		//	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(PrimitiveInitLightBindingUB,
		//		TVector<FPrimitivePtr>, Primitives, LinkedPrimitives,
		//		TVector<FLightPtr>, BindedLightResources, BindedLightResources,
		//		{
		//			BindLightResource_RenderThread(Primitives, BindedLightResources);
		//		});
		//}
	}
}
