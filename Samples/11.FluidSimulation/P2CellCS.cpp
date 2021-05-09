/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "P2CellCS.h"

FP2CellCS::FP2CellCS()
	: FComputeTask("S_P2CellCS")
{
}

FP2CellCS::~FP2CellCS()
{
}

void FP2CellCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
}

void FP2CellCS::UpdateComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InPbfParams,
	FUniformBufferPtr InBoundInfo,
	FUniformBufferPtr InPositions,
	FUniformBufferPtr InNumInCell,
	FUniformBufferPtr InCellParticles
)
{
	UBRef_PbfParams = InPbfParams;
	UBRef_BoundInfo = InBoundInfo;

	ResourceTable->PutUniformBufferInTable(InPositions, SRV_POSITIONS);
	ResourceTable->PutUniformBufferInTable(InNumInCell, UAV_NUM_IN_CELL);
	ResourceTable->PutUniformBufferInTable(InCellParticles, UAV_CELL_PARTICLES);
}

void FP2CellCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	const uint32 DispatchSize = (UBRef_PbfParams->GetElements() + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, UBRef_PbfParams);
	RHI->SetComputeConstantBuffer(1, UBRef_BoundInfo);
	RHI->SetComputeResourceTable(2, ResourceTable);
	RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
}