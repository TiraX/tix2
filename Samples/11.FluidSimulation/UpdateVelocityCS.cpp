/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "UpdateVelocityCS.h"

FUpdateVelocityCS::FUpdateVelocityCS()
	: FComputeTask("S_UpdateVelocityCS")
	, ThreadsCount(0)
{
}

FUpdateVelocityCS::~FUpdateVelocityCS()
{
}

void FUpdateVelocityCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
}

void FUpdateVelocityCS::UpdateComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InPbfParams,
	FUniformBufferPtr InBoundInfo,
	FUniformBufferPtr InPosOld,
	FUniformBufferPtr InPositions,
	FUniformBufferPtr InVelocities
)
{
	UBRef_PbfParams = InPbfParams;
	UBRef_BoundInfo = InBoundInfo;

	ResourceTable->PutUniformBufferInTable(InPosOld, SRV_POS_OLD);
	ResourceTable->PutRWUniformBufferInTable(InPositions, UAV_POSITIONS);
	ResourceTable->PutRWUniformBufferInTable(InVelocities, UAV_VELOCITIES);

	ThreadsCount = InPositions->GetElements();
}

void FUpdateVelocityCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	const uint32 DispatchSize = (ThreadsCount + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, UBRef_PbfParams);
	RHI->SetComputeConstantBuffer(1, UBRef_BoundInfo);
	RHI->SetComputeResourceTable(2, ResourceTable);
	RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
}