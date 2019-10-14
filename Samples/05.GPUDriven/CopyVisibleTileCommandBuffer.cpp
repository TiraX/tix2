/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "CopyVisibleTileCommandBuffer.h"
#include "SceneMetaInfos.h"

FCopyVisibleTileCommandBuffer::FCopyVisibleTileCommandBuffer()
	: FComputeTask("S_CopyVisibleTileCommandBuffer")
{
}

FCopyVisibleTileCommandBuffer::~FCopyVisibleTileCommandBuffer()
{
}

void FCopyVisibleTileCommandBuffer::PrepareResources(FRHI * RHI)
{
	CopyParams = ti_new FCopyCommandsParams;

	// Resource table for Compute cull shader
	ResourceTable = RHI->CreateRenderResourceTable(4, EHT_SHADER_RESOURCE);
	
	// Create counter reset
	CounterReset = ti_new FCounterReset;
	CounterReset->UniformBufferData[0].Zero = 0;
	CounterReset->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
}

void FCopyVisibleTileCommandBuffer::UpdateComputeArguments(
	FRHI * RHI,
	FScene * Scene,
	FUniformBufferPtr TileVisibleInfo,
	FUniformBufferPtr PrimitiveMetaInfo,
	FGPUCommandBufferPtr GPUCommandBuffer,
	FGPUCommandBufferPtr InProcessedGPUCommandBuffer)
{
	// Total commands count
	CopyParams->UniformBufferData[0].Info.X = (uint32)Scene->GetStaticDrawList(LIST_OPAQUE).size();
	CopyParams->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	// Set command buffer resources
	TI_TODO("Does this resource table, need to re-create?");
	// Set tile visible info
	ResourceTable->PutUniformBufferInTable(TileVisibleInfo, 0);
	// Set primitive meta info
	ResourceTable->PutUniformBufferInTable(PrimitiveMetaInfo, 1);
	// Set commands buffer
	ResourceTable->PutUniformBufferInTable(GPUCommandBuffer->GetCommandBuffer(), 2);
	// Set processed buffer UAV
	ResourceTable->PutUniformBufferInTable(InProcessedGPUCommandBuffer->GetCommandBuffer(), 3);
	ProcessedCommandBuffer = InProcessedGPUCommandBuffer;
}

void FCopyVisibleTileCommandBuffer::Run(FRHI * RHI)
{
	TI_ASSERT(0);
	//const uint32 BlockSize = 128;
	//const uint32 DispatchSize = MAX_DRAW_CALL_IN_SCENE / BlockSize;

	//// Reset command buffer counter
	//RHI->ComputeCopyBuffer(
	//	ProcessedCommandBuffer->GetCommandBuffer(), 
	//	ProcessedCommandBuffer->GetCommandBuffer()->GetCounterOffset(), 
	//	CounterReset->UniformBuffer, 
	//	0, 
	//	sizeof(uint32));

	//RHI->SetComputePipeline(ComputePipeline);
	//RHI->SetComputeBuffer(0, CopyParams->UniformBuffer);
	//RHI->SetComputeResourceTable(1, ResourceTable);

	//RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
}
