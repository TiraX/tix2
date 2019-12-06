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

void FGenerateClusterCullIndirectCommand::PrepareResources(FRHI * RHI, FUniformBufferPtr InClusterQueue, FGPUCommandBufferPtr DispatchCommandBuffer)
{
	ResourceTable = RHI->CreateRenderResourceTable(1, EHT_SHADER_RESOURCE);
	ResourceTable->PutUniformBufferInTable(DispatchCommandBuffer->GetCommandBuffer(), 0);
	ClustersQueue = InClusterQueue;
	CommandBuffer = DispatchCommandBuffer;

	// Create counter reset
	CounterReset = ti_new FCounterReset;
	CounterReset->UniformBufferData[0].Zero = 0;
	CounterReset->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
}

void FGenerateClusterCullIndirectCommand::Run(FRHI * RHI)
{
	RHI->SetResourceStateCB(CommandBuffer, RESOURCE_STATE_COPY_DEST);
	RHI->CopyBufferRegion(CommandBuffer->GetCommandBuffer(), CommandBuffer->GetCommandBuffer()->GetCounterOffset(), CounterReset->UniformBuffer, sizeof(uint32));

	RHI->SetResourceStateCB(CommandBuffer, RESOURCE_STATE_UNORDERED_ACCESS);

	RHI->SetComputePipeline(ComputePipeline);
	TI_ASSERT(ClustersQueue->GetCounterOffset() != 0);
	RHI->SetComputeBuffer(0, ClustersQueue, ClustersQueue->GetCounterOffset());
	RHI->SetComputeResourceTable(1, ResourceTable);

	RHI->DispatchCompute(vector3di(1, 1, 1), vector3di(1, 1, 1));
}
