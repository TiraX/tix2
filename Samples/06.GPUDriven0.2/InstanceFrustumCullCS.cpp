/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "InstanceFrustumCullCS.h"
#include "SceneMetaInfos.h"

FInstanceFrustumCullCS::FInstanceFrustumCullCS()
	: FComputeTask("S_InstanceFrustumCullCS")
{
}

FInstanceFrustumCullCS::~FInstanceFrustumCullCS()
{
}

void FInstanceFrustumCullCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(6, EHT_SHADER_RESOURCE);
}

void FInstanceFrustumCullCS::UpdataComputeParams(
	FUniformBufferPtr InFrustumUniform,
	FUniformBufferPtr InPrimitiveBBoxes,
	FUniformBufferPtr InInstanceMetaInfo,
	FInstanceBufferPtr InInstanceData,
	FGPUCommandBufferPtr InDrawCommandBuffer
)
{

}

void FInstanceFrustumCullCS::Run(FRHI * RHI)
{
}

