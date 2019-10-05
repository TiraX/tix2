/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "CopyVisibleTileCommandBuffer.h"

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
}

void FCopyVisibleTileCommandBuffer::UpdateComputeArguments(
	FRHI * RHI,
	FScene * Scene,
	FUniformBufferPtr TileVisibleInfo,
	FUniformBufferPtr PrimitiveMetaInfo,
	FGPUCommandBufferPtr CommandBuffer)
{
	// Total commands count
	CopyParams->UniformBufferData[0].Info.X = (uint32)Scene->GetStaticDrawList(LIST_OPAQUE).size();
	CopyParams->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	// Set command buffer resources
	TI_TODO("Does this resource table, need to re-create?");
	// Set tile visible info
	ResourceTable->PutBufferInTable(TileVisibleInfo, 0);
	// Set primitive meta info
	ResourceTable->PutBufferInTable(PrimitiveMetaInfo, 1);
	// Set commands buffer
	TI_ASSERT(0);
	// Set processed buffer UAV
}

void FCopyVisibleTileCommandBuffer::Run(FRHI * RHI)
{
}
