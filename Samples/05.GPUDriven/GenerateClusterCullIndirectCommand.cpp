/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GPUComputeUniforms.h"
#include "GenerateClusterCullIndirectCommand.h"
#include "SceneMetaInfos.h"

FGenerateClusterCullIndirectCommand::FGenerateClusterCullIndirectCommand()
	: FComputeTask("S_GenerateClusterCullIndirectCommandCS")
{
}

FGenerateClusterCullIndirectCommand::~FGenerateClusterCullIndirectCommand()
{
}

void FGenerateClusterCullIndirectCommand::PrepareResources(FRHI * RHI, FUniformBufferPtr ClustersLeft, FGPUCommandBufferPtr DispatchCommandBuffer)
{
	ResourceTable = RHI->CreateRenderResourceTable(2, EHT_SHADER_RESOURCE);
	ResourceTable->PutUniformBufferInTable(ClustersLeft, 0);
	ResourceTable->PutUniformBufferInTable(DispatchCommandBuffer->GetCommandBuffer(), 1);
}

void FGenerateClusterCullIndirectCommand::Run(FRHI * RHI)
{
	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeResourceTable(0, ResourceTable);

	RHI->DispatchCompute(vector3di(1, 1, 1), vector3di(1, 1, 1));
}
