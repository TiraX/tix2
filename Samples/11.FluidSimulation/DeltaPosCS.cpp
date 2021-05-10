/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "DeltaPosCS.h"

FDeltaPosCS::FDeltaPosCS()
	: FComputeTask("S_DeltaPosCS")
	, ThreadsCount(0)
{
}

FDeltaPosCS::~FDeltaPosCS()
{
}

void FDeltaPosCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
}

void FDeltaPosCS::UpdateComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InPbfParams,
	FUniformBufferPtr InBoundInfo,
	FUniformBufferPtr InNeighborNum,
	FUniformBufferPtr InNeighborParticles,
	FUniformBufferPtr InLambdas,
	FUniformBufferPtr InPositions
)
{
	UBRef_PbfParams = InPbfParams;
	UBRef_BoundInfo = InBoundInfo;

	ResourceTable->PutUniformBufferInTable(InNeighborNum, SRV_NEIGHBOR_NUM);
	ResourceTable->PutUniformBufferInTable(InNeighborParticles, SRV_NEIGHBOR_PARTICLES);
	ResourceTable->PutUniformBufferInTable(InLambdas, SRV_LAMBDAS);
	ResourceTable->PutRWUniformBufferInTable(InPositions, UAV_POSITIONS);

	ThreadsCount = InPositions->GetElements();
}

void FDeltaPosCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	const uint32 DispatchSize = (ThreadsCount + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, UBRef_PbfParams);
	RHI->SetComputeConstantBuffer(1, UBRef_BoundInfo);
	RHI->SetComputeResourceTable(2, ResourceTable);
	RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
}