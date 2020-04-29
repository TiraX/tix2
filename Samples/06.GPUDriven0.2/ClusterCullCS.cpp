/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ClusterCullCS.h"
#include "SceneMetaInfos.h"

FClusterCullCS::FClusterCullCS()
	: FComputeTask("S_ClusterCullCS")
{
}

FClusterCullCS::~FClusterCullCS()
{
}

void FClusterCullCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	TI_ASSERT(0);
}

void FClusterCullCS::UpdataComputeParams(
	FRHI * RHI
)
{
	TI_ASSERT(0);
}

void FClusterCullCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;

	TI_ASSERT(0);
}