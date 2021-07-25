/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GradientSubstractCS.h"
#include "FluidSolverGrid2d.h"

FGradientSubstractCS::FGradientSubstractCS()
	: FComputeTask("S_GradientSubstractCS")
{
}

FGradientSubstractCS::~FGradientSubstractCS()
{
}

void FGradientSubstractCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
}

void FGradientSubstractCS::UpdateComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InParam,
	FTexturePtr InPressure,
	FTexturePtr InVelocity
)
{
	UBRef_Fluid2dParam = InParam;

	ResourceTable->PutTextureInTable(InPressure, SRV_PRESSURE);
	ResourceTable->PutTextureInTable(InVelocity, SRV_VELOCITY);
	ResourceTable->PutRWTextureInTable(InVelocity, 0, UAV_VELOCITY);
}

void FGradientSubstractCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 16;
	const uint32 DispatchSize = FFluidSolverGrid2d::SimResolution / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, UBRef_Fluid2dParam);
	RHI->SetComputeResourceTable(1, ResourceTable);
	RHI->DispatchCompute(vector3di(BlockSize, BlockSize, 1), vector3di(DispatchSize, DispatchSize, 1));
}