/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "PCGSolver.h"
#include "FluidSimRenderer.h"


#define DO_PARALLEL (0)
const float eps = 1e-9f;

template<class T>
inline T AbsMax(const TVector<T>& Array)
{
	T Max = std::numeric_limits<T>::infinity();
	for (uint32 i = 0; i < (uint32)Array.size(); i++)
	{
		if (fabs(Array[i]) > Max)
		{
			Max = fabs(Array[i]);
		}
	}
	return Max;
}

FPCGSolver::FPCGSolver()
	: PressureTolerance(1e-8f)
	, MaxPCGIterations(200)
{
}

FPCGSolver::~FPCGSolver()
{
}

void FPCGSolver::Solve(PCGSolverParameters& Parameter)
{
	// Reset PressureGrid
	Parameter.Pressure->Clear();

	CollectFluidCells(Parameter);

	CalcNegativeDivergence(Parameter);
	if (AbsMax(NegativeDivergence) < PressureTolerance)
		return;

	BuildMatrixCoefficients(Parameter);

	CalcPreconditioner(Parameter);

	SolvePressure();

	// Copy values to pressure
	for (uint32 Index = 0; Index < (uint32)PressureGrids.size(); Index++)
	{
		const vector3di& GridIndex = PressureGrids[Index];
		Parameter.Pressure->Cell(GridIndex) = (float)PressureResult[Index];
	}
}

void FPCGSolver::CollectFluidCells(const PCGSolverParameters& Parameter)
{
	PressureGrids.clear();

	TI_ASSERT(Parameter.Marker->GetDimension() == Parameter.Pressure->GetDimension());
	const FFluidGrid3<int32>& Marker = *Parameter.Marker;

	// Collect 
	for (int32 Index = 0; Index < Parameter.Marker->GetTotalCells(); Index++)
	{
		if (Marker.Cell(Index) == 1)
		{
			vector3di GridIndex = Marker.ArrayIndexToGridIndex(Index);
			PressureGrids.push_back(GridIndex);
		}
	}

	GridsMap.reserve(PressureGrids.size());
	for (int32 Index = 0; Index < (int32)PressureGrids.size(); Index++)
	{
		const vector3di& GridIndex = PressureGrids[Index];
		GridsMap[GridIndex] = Index;
	}
}

void FPCGSolver::CalcNegativeDivergence(const PCGSolverParameters& Parameter)
{
	const FFluidGrid3<float>& U = *Parameter.U;
	const FFluidGrid3<float>& V = *Parameter.V;
	const FFluidGrid3<float>& W = *Parameter.W;
	vector3df InvCellSize = vector3df(1.f) / Parameter.CellSize;
	
	NegativeDivergence.clear();
	NegativeDivergence.resize(PressureGrids.size());

	for (uint32 Index = 0; Index < (uint32)PressureGrids.size(); Index++)
	{
		const vector3di& GridIndex = PressureGrids[Index];
		float ULeft = U.SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z);
		float URight = U.SafeCell(GridIndex.X + 1, GridIndex.Y, GridIndex.Z);
		float VBack = V.SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z);
		float VFront = V.SafeCell(GridIndex.X, GridIndex.Y + 1, GridIndex.Z);
		float WDown = W.SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z);
		float WUp = W.SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z + 1);

		float Div = (URight - ULeft) * InvCellSize.X + (VFront - VBack) * InvCellSize.Y + (WUp - WDown) * InvCellSize.Z;
		NegativeDivergence[Index] = -Div;
	}
}

void FPCGSolver::BuildMatrixCoefficients(const PCGSolverParameters& Parameter)
{
	MatrixCoeffients.clear();
	MatrixCoeffients.resize(PressureGrids.size());
	const FFluidGrid3<int32>& Marker = *Parameter.Marker;

	vector3df Scale = vector3df(Parameter.DeltaTime) / (Parameter.CellSize * Parameter.CellSize);
	for (uint32 Index = 0; Index < (uint32)PressureGrids.size(); Index++)
	{
		const vector3di& GridIndex = PressureGrids[Index];
		TI_ASSERT(Marker.Cell(GridIndex) == 1);

		// Right
		const int32 MarkerRight = Marker.Cell(GridIndex.X + 1, GridIndex.Y, GridIndex.Z);
		if (MarkerRight == 1)
		{
			MatrixCoeffients[Index].Diag += Scale.X;
			MatrixCoeffients[Index].PlusI -= Scale.X;
		}

		// Left
		const int32 MarkerLeft = Marker.Cell(GridIndex.X - 1, GridIndex.Y, GridIndex.Z);
		if (MarkerLeft == 1)
		{
			MatrixCoeffients[Index].Diag += Scale.X;
		}

		// Front
		const int32 MarkerFront = Marker.Cell(GridIndex.X, GridIndex.Y + 1, GridIndex.Z);
		if (MarkerFront == 1)
		{
			MatrixCoeffients[Index].Diag += Scale.Y;
			MatrixCoeffients[Index].PlusJ -= Scale.Y;
		}

		// Back
		const int32 MarkerBack = Marker.Cell(GridIndex.X, GridIndex.Y - 1, GridIndex.Z);
		if (MarkerBack == 1)
		{
			MatrixCoeffients[Index].Diag += Scale.Y;
		}

		// Up
		const int32 MarkerUp = Marker.Cell(GridIndex.X, GridIndex.Y, GridIndex.Z + 1);
		if (MarkerUp == 1)
		{
			MatrixCoeffients[Index].Diag += Scale.Z;
			MatrixCoeffients[Index].PlusK -= Scale.Z;
		}

		// Down
		const int32 MarkerDown = Marker.Cell(GridIndex.X, GridIndex.Y, GridIndex.Z - 1);
		if (MarkerDown == 1)
		{
			MatrixCoeffients[Index].Diag += Scale.Z;
		}
	}
}

void FPCGSolver::CalcPreconditioner(const PCGSolverParameters& Parameter)
{
	Precon.clear();
	Precon.resize(PressureGrids.size());

	const double Tau = 0.97;	// Tuning constant
	const double Sigma = 0.25;	// Safety constant

	for (uint32 Index = 0; Index < (uint32)PressureGrids.size(); Index++)
	{
		const vector3di& GridIndex = PressureGrids[Index];

		FGridsMap::iterator ItI = GridsMap.find(vector3di(GridIndex.X - 1, GridIndex.Y, GridIndex.Z));
		FGridsMap::iterator ItJ = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y - 1, GridIndex.Z));
		FGridsMap::iterator ItK = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y, GridIndex.Z - 1));

		double Diag = MatrixCoeffients[Index].Diag;

		double PlusI_I = ItI != GridsMap.end() ? MatrixCoeffients[ItI->second].PlusI : 0.f;
		double PlusI_J = ItJ != GridsMap.end() ? MatrixCoeffients[ItJ->second].PlusI : 0.f;
		double PlusI_K = ItK != GridsMap.end() ? MatrixCoeffients[ItK->second].PlusI : 0.f;

		double PlusJ_I = ItI != GridsMap.end() ? MatrixCoeffients[ItI->second].PlusJ : 0.f;
		double PlusJ_J = ItJ != GridsMap.end() ? MatrixCoeffients[ItJ->second].PlusJ : 0.f;
		double PlusJ_K = ItK != GridsMap.end() ? MatrixCoeffients[ItK->second].PlusJ : 0.f;

		double PlusK_I = ItI != GridsMap.end() ? MatrixCoeffients[ItI->second].PlusK : 0.f;
		double PlusK_J = ItJ != GridsMap.end() ? MatrixCoeffients[ItJ->second].PlusK : 0.f;
		double PlusK_K = ItK != GridsMap.end() ? MatrixCoeffients[ItK->second].PlusK : 0.f;

		double PreconI = ItI != GridsMap.end() ? Precon[ItI->second] : 0.f;
		double PreconJ = ItJ != GridsMap.end() ? Precon[ItJ->second] : 0.f;
		double PreconK = ItK != GridsMap.end() ? Precon[ItK->second] : 0.f;

		double V1 = PlusI_I * PreconI;
		double V2 = PlusJ_J * PreconJ;
		double V3 = PlusK_K * PreconK;
		double V4 = PreconI * PreconI;
		double V5 = PreconJ * PreconJ;
		double V6 = PreconK * PreconK;

		double e = 
			Diag - V1 * V1 - V2 * V2 - V3 * V3 - 
			Tau * (PlusI_I * (PlusJ_I + PlusK_I) * V4 +
				   PlusJ_J * (PlusI_J + PlusK_J) * V5 +
				   PlusK_K * (PlusI_K + PlusJ_K) * V6
				  );

		if (e < Sigma * Diag)
			e = Diag;

		if (fabs(e) > 10e-9)
		{
			Precon[Index] = 1.0 / sqrt(e);
		}
	}
}

void FPCGSolver::SolvePressure()
{
	TVector<double> Residual;
	Residual.resize(NegativeDivergence.size());
	for (uint32 Index = 0; Index < (uint32)NegativeDivergence.size(); Index++)
	{
		Residual[Index] = NegativeDivergence[Index];
	}

	TVector<double> Auxillary;
	Auxillary.resize(PressureGrids.size());
	ApplyPreconditioner(MatrixCoeffients, Precon, Residual, Auxillary);

	TVector<double> Search;
	Search = Auxillary;

	double Alpha = 0.0;
	double Beta = 0.0;
	double Sigma = DotVector(Auxillary, Residual);
	double SigmaNew = 0.0;
	int32 Iter = 0;

	PressureResult.clear();
	PressureResult.resize(PressureGrids.size());

	while (Iter < MaxPCGIterations)
	{
		ApplyMatrix(MatrixCoeffients, Search, Auxillary);
		Alpha = Sigma / DotVector(Auxillary, Search);
		AddScaledVector(PressureResult, Search, Alpha);
		AddScaledVector(Residual, Auxillary, -Alpha);

		double MaxCoeff = AbsMax(Residual);
		if (MaxCoeff < PressureTolerance)
		{
			_LOG(Log, "PCG Iter = %d, Error = %f.\n", Iter, MaxCoeff);
			return;
		}

		ApplyPreconditioner(MatrixCoeffients, Precon, Residual, Auxillary);
		SigmaNew = DotVector(Auxillary, Residual);
		Beta = SigmaNew / Sigma;
		AddScaledVector(Search, Search, Beta);
		AddScaledVector(Search, Auxillary, 1.0);
		Sigma = SigmaNew;

		Iter++;
	}
	_LOG(Log, "PCG MaxIteration(%d) Reached, Error = %f.\n", Iter, AbsMax(Residual));
}

void FPCGSolver::ApplyPreconditioner(const TVector<FMatrixCell>& A, const TVector<double>& PC, const TVector<double>& Residual, TVector<double>& Auxillary)
{
	// Solve A*q = residual
	TVector<double> Q;
	Q.resize(PressureGrids.size());
	for (uint32 Index = 0; Index < (uint32)PressureGrids.size(); Index++)
	{
		const vector3di& GridIndex = PressureGrids[Index];

		FGridsMap::iterator ItI = GridsMap.find(vector3di(GridIndex.X - 1, GridIndex.Y, GridIndex.Z));
		FGridsMap::iterator ItJ = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y - 1, GridIndex.Z));
		FGridsMap::iterator ItK = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y, GridIndex.Z - 1));

		double PlusI_I = 0.0;
		double PreconI = 0.0;
		double Q_I = 0.0;
		if (ItI != GridsMap.end())
		{
			PlusI_I = A[ItI->second].PlusI;
			PreconI = Precon[ItI->second];
			Q_I = Q[ItI->second];
		}

		double PlusJ_J = 0.0;
		double PreconJ = 0.0;
		double Q_J = 0.0;
		if (ItJ != GridsMap.end())
		{
			PlusJ_J = A[ItJ->second].PlusJ;
			PreconJ = Precon[ItJ->second];
			Q_J = Q[ItJ->second];
		}

		double PlusK_K = 0.0;
		double PreconK = 0.0;
		double Q_K = 0.0;
		if (ItK != GridsMap.end())
		{
			PlusK_K = A[ItK->second].PlusK;
			PreconK = Precon[ItK->second];
			Q_K = Q[ItK->second];
		}

		double T =
			double(Residual[Index]) -
			PlusI_I * PreconI * Q_I -
			PlusJ_J * PreconJ * Q_J -
			PlusK_K * PreconK * Q_K;

		T = T * Precon[Index];
		Q[Index] = T;
	}

	// Solve transpose(A)*z = q
	for (uint32 Index = (uint32)PressureGrids.size() - 1; Index >= 0; Index--)
	{
		const vector3di& GridIndex = PressureGrids[Index];

		FGridsMap::iterator ItI = GridsMap.find(vector3di(GridIndex.X + 1, GridIndex.Y, GridIndex.Z));
		FGridsMap::iterator ItJ = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y + 1, GridIndex.Z));
		FGridsMap::iterator ItK = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y, GridIndex.Z + 1));

		double VectI = ItI != GridsMap.end() ? Auxillary[ItI->second] : 0.0;
		double VectJ = ItJ != GridsMap.end() ? Auxillary[ItJ->second] : 0.0;
		double VectK = ItK != GridsMap.end() ? Auxillary[ItK->second] : 0.0;

		double PlusI = A[Index].PlusI;
		double PlusJ = A[Index].PlusJ;
		double PlusK = A[Index].PlusK;

		double PreconValue = PC[Index];
		double T = Q[Index] -
			PlusI * PreconValue * VectI -
			PlusJ * PreconValue * VectJ -
			PlusK * PreconValue * VectK;

		T = T * PreconValue;
		Auxillary[Index] = T;
	}
}

void FPCGSolver::ApplyMatrix(const TVector<FMatrixCell>& A, const TVector<double>& X, TVector<double>& Result)
{
	TI_ASSERT(A.size() == X.size() == Result.size());
	for (uint32 Index = 0; Index < (uint32)PressureGrids.size(); Index++)
	{
		const vector3di& GridIndex = PressureGrids[Index];

		FGridsMap::iterator It;
		double Val = 0.0;
		It = GridsMap.find(vector3di(GridIndex.X - 1, GridIndex.Y, GridIndex.Z));
		if (It != GridsMap.end())
			Val += X[It->second] * A[It->second].PlusI;

		It = GridsMap.find(vector3di(GridIndex.X + 1, GridIndex.Y, GridIndex.Z));
		if (It != GridsMap.end())
			Val += X[It->second] * A[Index].PlusI;

		It = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y - 1, GridIndex.Z));
		if (It != GridsMap.end())
			Val += X[It->second] * A[It->second].PlusJ;

		It = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y + 1, GridIndex.Z));
		if (It != GridsMap.end())
			Val += X[It->second] * A[Index].PlusJ;

		It = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y, GridIndex.Z - 1));
		if (It != GridsMap.end())
			Val += X[It->second] * A[It->second].PlusK;

		It = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y, GridIndex.Z + 1));
		if (It != GridsMap.end())
			Val += X[It->second] * A[Index].PlusK;

		Val += X[Index] * A[Index].Diag;

		Result[Index] = Val;
	}
}

void FPCGSolver::AddScaledVector(TVector<double>& V1, const TVector<double>& V2, double Scale)
{
	TI_ASSERT(V1.size() == V2.size());

	for (uint32 i = 0; i < (uint32)V1.size(); i++)
	{
		V1[i] += V2[i] * Scale;
	}
}

double FPCGSolver::DotVector(const TVector<double>& V1, const TVector<double>& V2)
{
	TI_ASSERT(V1.size() == V2.size());

	double Sum = 0.0;
	for (uint32 i = 0; i < (uint32)V1.size(); i++)
	{
		Sum += V1[i] * V2[i];
	}
	return Sum;
}