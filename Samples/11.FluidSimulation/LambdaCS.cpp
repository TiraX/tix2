/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "LambdaCS.h"

FLambdaCS::FLambdaCS()
	: FComputeTask("S_LambdaCS")
{
}

FLambdaCS::~FLambdaCS()
{
}

void FLambdaCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
}

void FLambdaCS::UpdateComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InPbfParams,
	FUniformBufferPtr InBoundInfo,
	FUniformBufferPtr InPositions,
	FUniformBufferPtr InNeighborNum,
	FUniformBufferPtr InNeighborParticles,
	FUniformBufferPtr InLambdas
)
{
	UBRef_PbfParams = InPbfParams;
	UBRef_BoundInfo = InBoundInfo;

	ResourceTable->PutUniformBufferInTable(InPositions, SRV_POSITIONS);
	ResourceTable->PutUniformBufferInTable(InNeighborNum, SRV_NEIGHBOR_NUM);
	ResourceTable->PutUniformBufferInTable(InNeighborParticles, SRV_NEIGHBOR_PARTICLES);
	ResourceTable->PutUniformBufferInTable(InLambdas, UAV_LAMBDAS);
}

void FLambdaCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	const uint32 DispatchSize = (UBRef_PbfParams->GetElements() + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, UBRef_PbfParams);
	RHI->SetComputeConstantBuffer(1, UBRef_BoundInfo);
	RHI->SetComputeResourceTable(2, ResourceTable);
	RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
}