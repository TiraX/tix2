/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ApplyGravityCS.h"

FApplyGravityCS::FApplyGravityCS()
	: FComputeTask("S_ApplyGravityCS")
{
}

FApplyGravityCS::~FApplyGravityCS()
{
}

void FApplyGravityCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
}

void FApplyGravityCS::UpdateComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InPbfParams,
	FUniformBufferPtr InBoundInfo,
	FUniformBufferPtr InPositions,
	FUniformBufferPtr InVelocities,
	FUniformBufferPtr InPosOld
)
{
	UBRef_PbfParams = InPbfParams;
	UBRef_BoundInfo = InBoundInfo;

	ResourceTable->PutUniformBufferInTable(InPositions, UAV_POSITIONS);
	ResourceTable->PutUniformBufferInTable(InVelocities, SRV_VELOCITIES);
	ResourceTable->PutUniformBufferInTable(InPosOld, UAV_POS_OLD);
}

void FApplyGravityCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	const uint32 DispatchSize = (UBRef_PbfParams->GetElements() + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, UBRef_PbfParams);
	RHI->SetComputeConstantBuffer(1, UBRef_BoundInfo);
	RHI->SetComputeResourceTable(2, ResourceTable);
	RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
}