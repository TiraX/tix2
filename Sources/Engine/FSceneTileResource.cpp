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

	void FSceneTileResource::CreateBLASAndInstances(FMeshBufferPtr MB, TInstanceBufferPtr InInstanceData, const int32 InstanceCount, const int32 InstanceOffset)
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
			const FFloat4* InstanceData = (const FFloat4*)InInstanceData->GetInstanceData();
			for (int32 Ins = 0; Ins < InstanceCount; ++Ins)
			{
				const FFloat4* InsData = InstanceData + (InstanceOffset + Ins) * 4;
				FMatrix3x4 Mat3x4;
				Mat3x4.SetTranslation(vector3df(InsData[0].X, InsData[0].Y, InsData[0].Z));
				Mat3x4[0] = InsData[1].X;
				Mat3x4[1] = InsData[1].Y;
				Mat3x4[2] = InsData[1].Z;

				Mat3x4[4] = InsData[2].X;
				Mat3x4[5] = InsData[2].Y;
				Mat3x4[6] = InsData[2].Z;

				Mat3x4[8] = InsData[3].X;
				Mat3x4[9] = InsData[3].Y;
				Mat3x4[10] = InsData[3].Z;

				SceneTileBLASInstances[MB].push_back(Mat3x4);

			}
		}
	}

	void FSceneTileResource::BuildBLAS()
	{
		for (THMap<FMeshBufferPtr, FBottomLevelAccelerationStructurePtr>::iterator it = SceneTileBLASes.begin(); it != SceneTileBLASes.end(); it ++)
		{
			it->second->Build();
		}
		FRenderThread::Get()->GetRenderScene()->SetSceneFlag(FScene::SceneBLASDirty);
	}
}