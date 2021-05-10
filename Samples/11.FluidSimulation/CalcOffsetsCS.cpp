/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "CalcOffsetsCS.h"

FCalcOffsetsCS::FCalcOffsetsCS()
	: FComputeTask("S_CalcOffsetsCS")
{
}

FCalcOffsetsCS::~FCalcOffsetsCS()
{
}

void FCalcOffsetsCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
}

void FCalcOffsetsCS::UpdateComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InPbfParams,
	FUniformBufferPtr InBoundInfo,
	FUniformBufferPtr InNumInCell,
	FUniformBufferPtr InCellParticleOffsets
)
{
	UBRef_PbfParams = InPbfParams;
	UBRef_BoundInfo = InBoundInfo;

	ResourceTable->PutRWUniformBufferInTable(InNumInCell, UAV_NUM_IN_CELL);
	ResourceTable->PutRWUniformBufferInTable(InCellParticleOffsets, UAV_CELL_PARTICLE_OFFSETS);
}

void FCalcOffsetsCS::Run(FRHI * RHI)
{
	//const uint32 BlockSize = 128;
	//const uint32 DispatchSize = (UBRef_PbfParams->GetElements() + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, UBRef_PbfParams);
	RHI->SetComputeConstantBuffer(1, UBRef_BoundInfo);
	RHI->SetComputeResourceTable(2, ResourceTable);
	RHI->DispatchCompute(vector3di(1, 1, 1), vector3di(1, 1, 1));
}