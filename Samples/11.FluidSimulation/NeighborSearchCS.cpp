/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "NeighborSearchCS.h"

FNeighborSearchCS::FNeighborSearchCS()
	: FComputeTask("S_NeighborSearchCS")
{
}

FNeighborSearchCS::~FNeighborSearchCS()
{
}

void FNeighborSearchCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
}

void FNeighborSearchCS::UpdateComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InPbfParams,
	FUniformBufferPtr InBoundInfo,
	FUniformBufferPtr InNumInCell,
	FUniformBufferPtr InCellParticleOffsets,
	FUniformBufferPtr InPositions,
	FUniformBufferPtr InNeighborNum,
	FUniformBufferPtr InNeighborParticles
)
{
	UBRef_PbfParams = InPbfParams;
	UBRef_BoundInfo = InBoundInfo;

	ResourceTable->PutUniformBufferInTable(InNumInCell, SRV_NUM_IN_CELL);
	ResourceTable->PutUniformBufferInTable(InCellParticleOffsets, SRV_CELL_PARTICLE_OFFSETS);
	ResourceTable->PutUniformBufferInTable(InPositions, SRV_POSITIONS);
	ResourceTable->PutUniformBufferInTable(InNeighborNum, UAV_NEIGHBOR_NUM);
	ResourceTable->PutUniformBufferInTable(InNeighborParticles, UAV_NEIGHBOR_PARTICLES);
}

void FNeighborSearchCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	const uint32 DispatchSize = (UBRef_PbfParams->GetElements() + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, UBRef_PbfParams);
	RHI->SetComputeConstantBuffer(1, UBRef_BoundInfo);
	RHI->SetComputeResourceTable(2, ResourceTable);
	RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
}