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

void FGenerateClusterCullIndirectCommand::PrepareResources(FRHI * RHI)
{
	TI_ASSERT(0);
}

void FGenerateClusterCullIndirectCommand::UpdateComputeArguments(
	FRHI * RHI,
	FSceneMetaInfos * SceneSceneMetaInfos,
	FUniformBufferPtr InstanceVisibleInfo,
	FUniformBufferPtr InstanceMetaInfo,
	FGPUCommandBufferPtr GPUCommandBuffer,
	FGPUCommandBufferPtr InProcessedGPUCommandBuffer)
{
	TI_ASSERT(0);
}

void FGenerateClusterCullIndirectCommand::Run(FRHI * RHI)
{
	TI_ASSERT(0);
}
