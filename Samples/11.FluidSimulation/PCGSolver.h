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
	FFluidGrid3<int32>* Marker;

	FFluidGrid3<float>* Pressure;

	PCGSolverParameters()
		: DeltaTime(0.1f)
		, U(nullptr)
		, V(nullptr)
		, W(nullptr)
		, Marker(nullptr)
		, Pressure(nullptr)
	{}
};

struct FMatrixCell
{
	float Diag;
	float PlusI;
	float PlusJ;
	float PlusK;

	FMatrixCell()
		: Diag(0.f)
		, PlusI(0.f)
		, PlusJ(0.f)
		, PlusK(0.f)
	{}
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
	void CollectFluidCells(const PCGSolverParameters& Parameter);
	void CalcNegativeDivergence(const PCGSolverParameters& Parameter);
	void BuildMatrixCoefficients(const PCGSolverParameters& Parameter);
	void CalcPreconditioner(const PCGSolverParameters& Parameter);

	void SolvePressure();

	void ApplyPreconditioner(const TVector<FMatrixCell>& A, const TVector<double>& PC, const TVector<double>& Residual, TVector<double>& Auxillary);
	void ApplyMatrix(const TVector<FMatrixCell>& A, const TVector<double>& X, TVector<double>& Result);
	void AddScaledVector(TVector<double>& V1, const TVector<double>& V2, double Scale);
	double DotVector(const TVector<double>& V1, const TVector<double>& V2);

private:
	float PressureTolerance;
	int32 MaxPCGIterations;
	TVector<vector3di> PressureGrids;
	typedef THMap<vector3di, int32> FGridsMap;
	FGridsMap GridsMap;
	TVector<float> NegativeDivergence;

	TVector<FMatrixCell> MatrixCoeffients;
	TVector<double> Precon;
	TVector<double> PressureResult;
};
