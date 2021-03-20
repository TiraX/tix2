/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GaussRandomCS.h"

static const int RandTexSize = 64;

FGaussRandomCS::FGaussRandomCS()
	: FComputeTask("S_GaussRandomCS")
{
}

FGaussRandomCS::~FGaussRandomCS()
{
}

void FGaussRandomCS::PrepareResources(FRHI * RHI)
{
}

void FGaussRandomCS::UpdataComputeParams(
	FRHI * RHI
)
{
}

void FGaussRandomCS::Run(FRHI * RHI)
{
	//const uint32 BlockSize = 8;
	//int32 RTW = RHI->GetViewport().Width;
	//int32 RTH = RHI->GetViewport().Height;

	//vector3di DispatchSize = vector3di(1, 1, 1);
	//DispatchSize.X = (RTW + BlockSize - 1) / BlockSize;
	//DispatchSize.Y = (RTH + BlockSize - 1) / BlockSize;

	//RHI->SetComputePipeline(ComputePipeline);
	//RHI->SetComputeConstantBuffer(0, InfoUniform->UniformBuffer);
	//RHI->SetComputeResourceTable(1, ResourceTable);

	//RHI->DispatchCompute(vector3di(BlockSize, BlockSize, 1), DispatchSize);
}