/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ClearPressureCS.h"
#include "FluidSolverGrid2d.h"

FClearPressureCS::FClearPressureCS()
	: FComputeTask("S_ClearPressureCS")
{
}

FClearPressureCS::~FClearPressureCS()
{
}

void FClearPressureCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
}

void FClearPressureCS::UpdateComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InParam,
	FTexturePtr InPressure
)
{
	UBRef_Fluid2dParam = InParam;

	ResourceTable->PutTextureInTable(InPressure, SRV_PRESSURE);
	ResourceTable->PutRWTextureInTable(InPressure, 0, UAV_PRESSURE);
}

void FClearPressureCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 16;
	const uint32 DispatchSize = FFluidSolverGrid2d::SimResolution / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, UBRef_Fluid2dParam);
	RHI->SetComputeResourceTable(1, ResourceTable);
	RHI->DispatchCompute(vector3di(BlockSize, BlockSize, 1), vector3di(DispatchSize, DispatchSize, 1));
}