/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ApplyDeltaPosCS.h"

FApplyDeltaPosCS::FApplyDeltaPosCS()
	: FComputeTask("S_ApplyDeltaPosCS")
	, ThreadsCount(0)
{
}

FApplyDeltaPosCS::~FApplyDeltaPosCS()
{
}

void FApplyDeltaPosCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
}

void FApplyDeltaPosCS::UpdateComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InPbfParams,
	FUniformBufferPtr InBoundInfo,
	FUniformBufferPtr InDeltaPosition,
	FUniformBufferPtr InPositions
)
{
	UBRef_PbfParams = InPbfParams;
	UBRef_BoundInfo = InBoundInfo;

	ResourceTable->PutUniformBufferInTable(InDeltaPosition, SRV_DELTA_POS);
	ResourceTable->PutRWUniformBufferInTable(InPositions, UAV_POSITIONS);

	ThreadsCount = InPositions->GetElements();
}

void FApplyDeltaPosCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	const uint32 DispatchSize = (ThreadsCount + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, UBRef_PbfParams);
	RHI->SetComputeConstantBuffer(1, UBRef_BoundInfo);
	RHI->SetComputeResourceTable(2, ResourceTable);
	RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
}