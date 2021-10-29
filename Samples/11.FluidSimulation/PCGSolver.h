/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FluidSolver.h"
#include "FluidParticle.h"
#include "FluidGrid.h"

//typedef double pcg_float;
typedef float pcg_float;

struct PCGSolverParameters {
    vector3df CellSize;
    float DeltaTime;

	FFluidGrid3<float>* U;
	FFluidGrid3<float>* V;
	FFluidGrid3<float>* W;
	FFluidGrid3<float>* UW;
	FFluidGrid3<float>* VW;
	FFluidGrid3<float>* WW;
	//FFluidGrid3<int32>* Marker;
	FFluidGrid3<float>* LiquidSDF;


	FFluidGrid3<float>* Pressure;

	PCGSolverParameters()
		: DeltaTime(0.1f)
		, U(nullptr)
		, V(nullptr)
		, W(nullptr)
		, UW(nullptr)
		, VW(nullptr)
		, WW(nullptr)
		//, Marker(nullptr)
		, LiquidSDF(nullptr)
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

	void ApplyPreconditioner(const TVector<FMatrixCell>& A, const TVector<pcg_float>& PC, const TVector<pcg_float>& Residual, TVector<pcg_float>& Auxillary);
	void ApplyMatrix(const TVector<FMatrixCell>& A, const TVector<pcg_float>& X, TVector<pcg_float>& Result);
	void AddScaledVector(TVector<pcg_float>& V1, const TVector<pcg_float>& V2, pcg_float Scale);
	void AddScaledVectors(const TVector<pcg_float>& V1, pcg_float S1, const TVector<pcg_float>& V2, pcg_float S2, TVector<pcg_float>& Result);
	pcg_float DotVector(const TVector<pcg_float>& V1, const TVector<pcg_float>& V2);

private:
	float PressureTolerance;
	int32 MaxPCGIterations;
	TVector<vector3di> PressureGrids;
	typedef THMap<vector3di, int32> FGridsMap;
	FGridsMap GridsMap;
	TVector<pcg_float> NegativeDivergence;

	TVector<FMatrixCell> A;
	TVector<pcg_float> Precon;
	TVector<pcg_float> PressureResult;
};
