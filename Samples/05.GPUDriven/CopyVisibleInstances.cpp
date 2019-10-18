/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "CopyVisibleInstances.h"
#include "SceneMetaInfos.h"

FCopyVisibleInstances::FCopyVisibleInstances()
	: FComputeTask("S_CopyVisibleInstances")
{
}

FCopyVisibleInstances::~FCopyVisibleInstances()
{
}

void FCopyVisibleInstances::PrepareResources(FRHI * RHI)
{
	CopyParams = ti_new FCopyCommandsParams;

	// Resource table for Compute cull shader
	ResourceTable = RHI->CreateRenderResourceTable(4, EHT_SHADER_RESOURCE);
	
	// Create counter reset
	CounterReset = ti_new FCounterReset;
	CounterReset->UniformBufferData[0].Zero = 0;
	CounterReset->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
}

void FCopyVisibleInstances::UpdateComputeArguments(
	FRHI * RHI,
	FSceneMetaInfos * SceneSceneMetaInfos,
	FUniformBufferPtr InstanceVisibleInfo,
	FUniformBufferPtr InstanceMetaInfo,
	FGPUCommandBufferPtr GPUCommandBuffer,
	FGPUCommandBufferPtr InProcessedGPUCommandBuffer)
{
	// Total commands count
	CopyParams->UniformBufferData[0].Info.X = SceneSceneMetaInfos->GetSceneInstancesAdded();
	CopyParams->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	TI_ASSERT(InstanceMetaInfo->GetElements() == InstanceVisibleInfo->GetElements() &&
		InstanceMetaInfo->GetElements() == InProcessedGPUCommandBuffer->GetEncodedCommandsCount());
	// Set command buffer resources
	TI_TODO("Does this resource table, need to re-create?");
	// Set tile visible info
	ResourceTable->PutUniformBufferInTable(InstanceVisibleInfo, 0);
	// Set primitive meta info
	ResourceTable->PutUniformBufferInTable(InstanceMetaInfo, 1);
	// Set commands buffer
	ResourceTable->PutUniformBufferInTable(GPUCommandBuffer->GetCommandBuffer(), 2);
	// Set processed buffer UAV
	ResourceTable->PutUniformBufferInTable(InProcessedGPUCommandBuffer->GetCommandBuffer(), 3);
	ProcessedCommandBuffer = InProcessedGPUCommandBuffer;
}

void FCopyVisibleInstances::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	const uint32 DispatchSize = ProcessedCommandBuffer->GetEncodedCommandsCount() / BlockSize;

	// Reset command buffer counter
	RHI->ComputeCopyBuffer(
		ProcessedCommandBuffer->GetCommandBuffer(), 
		ProcessedCommandBuffer->GetCommandBuffer()->GetCounterOffset(), 
		CounterReset->UniformBuffer, 
		0, 
		sizeof(uint32));

	RHI->SetComputePipeline(ComputePipeline);
	//RHI->SetComputeBuffer(0, CopyParams->UniformBuffer);
	RHI->SetComputeResourceTable(0, ResourceTable);

	RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
}
