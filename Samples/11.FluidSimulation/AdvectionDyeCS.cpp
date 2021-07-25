/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "AdvectionDyeCS.h"
#include "FluidSolverGrid2d.h"

FAdvectionDyeCS::FAdvectionDyeCS()
	: FComputeTask("S_AdvectionDyeCS")
{
}

FAdvectionDyeCS::~FAdvectionDyeCS()
{
}

void FAdvectionDyeCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
}

void FAdvectionDyeCS::UpdateComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InParam,
	FTexturePtr InVelocity,
	FTexturePtr InDye
)
{
	UBRef_Fluid2dParam = InParam;

	ResourceTable->PutTextureInTable(InVelocity, SRV_VELOCITY);
	ResourceTable->PutTextureInTable(InDye, SRV_DYE_SRC);
	ResourceTable->PutRWTextureInTable(InDye, 0, UAV_DYE_DST);
}

void FAdvectionDyeCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 16;
	const uint32 DispatchSize = FFluidSolverGrid2d::DyeResolution / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, UBRef_Fluid2dParam);
	RHI->SetComputeResourceTable(1, ResourceTable);
	RHI->DispatchCompute(vector3di(BlockSize, BlockSize, 1), vector3di(DispatchSize, DispatchSize, 1));
}