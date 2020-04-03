/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "InstanceOccludeCullCS.h"
#include "SceneMetaInfos.h"

FInstanceOccludeCullCS::FInstanceOccludeCullCS()
	: FComputeTask("S_InstanceOccludeCullCS")
{
}

FInstanceOccludeCullCS::~FInstanceOccludeCullCS()
{
}

void FInstanceOccludeCullCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(6, EHT_SHADER_RESOURCE);
}

void FInstanceOccludeCullCS::UpdataComputeParams(
	FRHI * RHI
)
{
}

void FInstanceOccludeCullCS::Run(FRHI * RHI)
{
}

