/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "OcclusionDispatchCmd.h"
#include "SceneMetaInfos.h"

FOcclusionDispatchCmdCS::FOcclusionDispatchCmdCS()
	: FComputeTask("S_OcclusionDispatchCmdCS")
{
}

FOcclusionDispatchCmdCS::~FOcclusionDispatchCmdCS()
{
}

void FOcclusionDispatchCmdCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
	
	DispatchThreadCount = RHI->CreateUniformBuffer(sizeof(uint32) * 4, 1, UB_FLAG_COMPUTE_WRITABLE);
	DispatchThreadCount->SetResourceName("DispatchThreadCount");
	RHI->UpdateHardwareResourceUB(DispatchThreadCount, nullptr);
	ResourceTable->PutUniformBufferInTable(DispatchThreadCount, PARAM_DISPATCH_THREAD_COUNT);
}

void FOcclusionDispatchCmdCS::UpdataComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InVisibleInstanceCount
)
{
	if (VisibleInstanceCount != InVisibleInstanceCount)
	{
		VisibleInstanceCount = InVisibleInstanceCount;
	}
}

void FOcclusionDispatchCmdCS::Run(FRHI * RHI)
{
	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, VisibleInstanceCount);
	RHI->SetComputeResourceTable(1, ResourceTable);
	RHI->DispatchCompute(vector3di(1, 1, 1), vector3di(1, 1, 1));
}

