/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TNodeStaticMesh.h"

namespace tix
{
	TStaticMeshLoadingFinishDelegate::TStaticMeshLoadingFinishDelegate(
		const TString& InLevelName, 
		const vector2di& InSceneTilePos, 
		int32 InMeshIndex)
		: LevelName(InLevelName)
		, SceneTilePos(InSceneTilePos)
		, MeshIndexInTile(InMeshIndex)
	{}

	TStaticMeshLoadingFinishDelegate::~TStaticMeshLoadingFinishDelegate()
	{}

	void TStaticMeshLoadingFinishDelegate::LoadingFinished(TAssetPtr InAsset)
	{
		TI_ASSERT(IsGameThread());

		// Find the scene tile node it belongs to.
		int8 SceneTilePath[128];
		sprintf(SceneTilePath, "%s.t%d_%d", LevelName.c_str(), SceneTilePos.X, SceneTilePos.Y);
		TNode * NodeFromPath = TEngine::Get()->GetScene()->GetRoot()->GetNodeByPath(SceneTilePath);

		if (NodeFromPath != nullptr)
		{
			TI_ASSERT(NodeFromPath->GetType() == ENT_SceneTile);
			TNodeSceneTile * NodeSceneTile = static_cast<TNodeSceneTile*>(NodeFromPath);

			// Mesh asset load finished add it to TScene
			const TVector<TResourcePtr>& Resources = InAsset->GetResources();
			TI_ASSERT(Resources.size() > 0);
			TResourcePtr FirstResource = Resources[0];
			TI_ASSERT(FirstResource->GetType() == ERES_MESH);
			TAssetPtr MeshRes = InAsset;

			// Get Instance Buffer from SceneTile
			TI_ASSERT(MeshIndexInTile >= 0);
			TInstanceBufferPtr InstanceBuffer = NodeSceneTile->GetInstanceBufferByIndex(MeshIndexInTile);

			// Gather loaded mesh resources
			TVector<TMeshBufferPtr> Meshes;
			Meshes.reserve(MeshRes->GetResources().size());
			for (auto Res : MeshRes->GetResources())
			{
				TMeshBufferPtr Mesh = static_cast<TMeshBuffer*>(Res.get());
				Meshes.push_back(Mesh);
			}

			// Create static mesh node
			TNodeStaticMesh * NodeStaticMesh = TNodeFactory::CreateNode<TNodeStaticMesh>(NodeSceneTile, MeshRes->GetName());
			NodeStaticMesh->LinkMesh(Meshes, InstanceBuffer, false, false);
		}
	}

	//////////////////////////////////////////////////////////////////////////

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
			Primitive->SetMesh(M->MeshBufferResource, M->GetBBox(), M->GetDefaultMaterial(), InInstanceBuffer != nullptr ? InInstanceBuffer->InstanceResource : nullptr);
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

			// Init uniform buffer resource in render thread
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(UpdatePrimitiveBuffer,
				TVector<FPrimitivePtr>, Primitives, LinkedPrimitives,
				matrix4, WorldTransform, AbsoluteTransformation,
				{
					for (auto P : Primitives)
					{
						P->SetWorldTransform(WorldTransform);
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
