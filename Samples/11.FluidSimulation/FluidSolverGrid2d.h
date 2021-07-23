/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FluidSolver.h"

class FFluidSolverGrid2d : public FFluidSolver
{
public:
	FFluidSolverGrid2d();
	virtual ~FFluidSolverGrid2d();

	virtual void CreateGrid(int32 Size, int32 DyeSize) override;
	virtual void Sim(FRHI* RHI, float Dt) override;
	virtual void RenderGrid(FRHI* RHI, FScene* Scene) override;

private:

private:
	// Compute Tasks
};
