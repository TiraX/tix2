/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSolverGrid2d.h"
#include "FluidSimRenderer.h"

FFluidSolverGrid2d::FFluidSolverGrid2d()
{
	SubStep = 1;
}

FFluidSolverGrid2d::~FFluidSolverGrid2d()
{
}

void FFluidSolverGrid2d::CreateGrid(int32 Size, int32 DyeSize)
{
	TI_ASSERT(0);
}

void FFluidSolverGrid2d::Sim(FRHI * RHI, float Dt)
{
	RHI->BeginComputeTask();

	RHI->EndComputeTask();
}

void FFluidSolverGrid2d::RenderGrid(FRHI* RHI, FScene* Scene)
{
	TI_ASSERT(0);
}