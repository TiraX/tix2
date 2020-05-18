/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ClusterDispatchCmd.h"
#include "SceneMetaInfos.h"

FClusterDispatchCmdCS::FClusterDispatchCmdCS()
	: FComputeTask("S_ClusterDispatchCmdCS")
{
}

FClusterDispatchCmdCS::~FClusterDispatchCmdCS()
{
}

void FClusterDispatchCmdCS::PrepareResources(FRHI * RHI, const TString& InName)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
	
	DispatchThreadCount = RHI->CreateUniformBuffer(sizeof(uint32) * 4, 1, UB_FLAG_COMPUTE_WRITABLE);
	DispatchThreadCount->SetResourceName(InName);
	RHI->UpdateHardwareResourceUB(DispatchThreadCount, nullptr);
	ResourceTable->PutUniformBufferInTable(DispatchThreadCount, PARAM_DISPATCH_THREAD_COUNT);
}

void FClusterDispatchCmdCS::UpdataComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InVisibleInstanceCount
)
{
	if (VisibleInstanceCount != InVisibleInstanceCount)
	{
		VisibleInstanceCount = InVisibleInstanceCount;
	}
}

void FClusterDispatchCmdCS::Run(FRHI * RHI)
{
	RHI->SetResourceStateUB(DispatchThreadCount, RESOURCE_STATE_UNORDERED_ACCESS);
	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, VisibleInstanceCount);
	RHI->SetComputeResourceTable(1, ResourceTable);
	RHI->DispatchCompute(vector3di(1, 1, 1), vector3di(1, 1, 1));
}

