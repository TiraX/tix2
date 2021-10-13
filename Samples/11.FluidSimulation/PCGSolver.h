/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FluidSolver.h"
#include "FluidParticle.h"
#include "FluidGrid.h"

struct PCGSolverParameters {
    vector3df CellSize;
    float DeltaTime;

	FFluidGrid3<float>* U;
	FFluidGrid3<float>* V;
	FFluidGrid3<float>* W;

	FFluidGrid3<float>* Pressure;
};

// PCG solver for FLIP
// With MIC (Modified incompolete cholesky preconditioner)
class FPCGSolver
{
public:

	FPCGSolver();
	~FPCGSolver();

	void Solve(PCGSolverParameters& Parameter);

private:
	void CalcNegativeDivergence(const FFluidGrid3<float>& U, const FFluidGrid3<float>& V, const FFluidGrid3<float>& W);

private:
};
