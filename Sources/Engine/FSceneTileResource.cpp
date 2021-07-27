/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FSceneTileResource.h"

namespace tix
{
	FSceneTileResource::FSceneTileResource()
		: FRenderResource(RRT_SCENE_TILE)
		, TotalStaticMeshes(0)
		, TotalSMSections(0)
		, TotalSMInstances(0)
	{
		TI_ASSERT(0);
	}

	FSceneTileResource::FSceneTileResource(const TSceneTileResource& InSceneTileResource)
		: FRenderResource(RRT_SCENE_TILE)
		, Position(InSceneTileResource.Position)
		, BBox(InSceneTileResource.BBox)
		, TotalStaticMeshes(InSceneTileResource.SMInfos.NumMeshes)
		, TotalSMSections(InSceneTileResource.SMInfos.TotalSections)
		, TotalSMInstances(InSceneTileResource.SMInstances.NumInstances)
		, InstanceCountAndOffset(InSceneTileResource.SMInstances.InstanceCountAndOffset)
	{
		TI_ASSERT(IsGameThread());
		InstanceBuffer = InSceneTileResource.SMInstances.InstanceBuffer->InstanceResource;

		Primitives.resize(TotalSMSections);
	}

	FSceneTileResource::~FSceneTileResource()
	{
	}

	void FSceneTileResource::AddPrimitive(uint32 Index, FPrimitivePtr Primitive)
	{
		Primitives[Index] = Primitive;
		FRenderThread::Get()->GetRenderScene()->SetSceneFlag(FScene::ScenePrimitivesDirty);
	}

	void FSceneTileResource::AppendPrimitive(FPrimitivePtr Primitive)
	{
		// Temp use, wait for refactor
		Primitives.push_back(Primitive);
		FRenderThread::Get()->GetRenderScene()->SetSceneFlag(FScene::ScenePrimitivesDirty);
	}

	void FSceneTileResource::CreateBLASAndInstances(FMeshBufferPtr MB, TInstanceBufferPtr InstanceData, const int32 InstanceCount, const int32 InstanceOffset)
	{
		if (FRHI::RHIConfig.IsRaytracingEnabled())
		{
			FBottomLevelAccelerationStructurePtr BLAS = nullptr;

			// Find BLAS for this mesh, if not found, create ONE.
			THMap<FMeshBufferPtr, FBottomLevelAccelerationStructurePtr>::iterator Found = SceneTileBLASes.find(MB);
			if (Found == SceneTileBLASes.end())
			{
				BLAS = FRHI::Get()->CreateBottomLevelAccelerationStructure();

				int8 Name[128];
				sprintf_s(Name, 128, "BLAS_%d_%d-%s",
					Position.X,
					Position.Y,
					MB->GetResourceName().c_str());
				BLAS->SetResourceName(Name);
				BLAS->AddMeshBuffer(MB);

				SceneTileBLASes[MB] = BLAS;
			}
			else
			{
				BLAS = Found->second;
			}

			// Create BLAS instances
			TI_ASSERT(0);
		}
	}

	void FSceneTileResource::BuildBLAS()
	{
		for (THMap<FMeshBufferPtr, FBottomLevelAccelerationStructurePtr>::iterator it = SceneTileBLASes.begin(); it != SceneTileBLASes.end(); it ++)
		{
			it->second->Build();
		}
	}
}