/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "PCGSolver.h"
#include "FluidSimRenderer.h"
#include "GeometrySDF.h"
#include "LevelsetUtils.h"

#define DO_PARALLEL (0)
const float eps = 1e-9f;

static float DebugFloat[3];
static double DebugDouble[3];

void GetDebugInfo(const TVector<FMatrixCell>& A)
{
#ifdef TIX_DEBUG
	float Min = std::numeric_limits<float>::infinity();
	float Max = -std::numeric_limits<float>::infinity();

	float Sum = 0.f;
	for (const auto& a : A)
	{
		Sum += a.Diag;
		if (a.Diag < Min)
			Min = a.Diag;
		if (a.Diag > Max)
			Max = a.Diag;
	}
	DebugFloat[0] = Min;
	DebugFloat[1] = Max;
	DebugFloat[2] = Sum / (float)(A.size());
#endif
}

inline void GetDebugInfo(const TVector<float>& A)
{
#ifdef TIX_DEBUG
	float Min = std::numeric_limits<float>::infinity();
	float Max = -std::numeric_limits<float>::infinity();

	float Sum = 0;
	for (const auto& a : A)
	{
		Sum += a;
		if (a < Min)
			Min = a;
		if (a > Max)
			Max = a;
	}
	DebugFloat[0] = Min;
	DebugFloat[1] = Max;
	DebugFloat[2] = Sum / (float)(A.size());
#endif
}
inline void GetDebugInfo(const TVector<double>& A)
{
#ifdef TIX_DEBUG
	double Min = std::numeric_limits<double>::infinity();
	double Max = -std::numeric_limits<double>::infinity();

	double Sum = 0;
	for (const auto& a : A)
	{
		Sum += a;
		if (a < Min)
			Min = a;
		if (a > Max)
			Max = a;
	}
	DebugDouble[0] = Min;
	DebugDouble[1] = Max;
	DebugDouble[2] = Sum / (double)(A.size());
#endif
}


double DebugNegative[3];
inline void GetDebugNegative(const TVector<double>& A)
{
	int Count = 0;
	double Sum = 0.f;
	for (const auto& a : A)
	{
		if (a < 0)
		{
			Sum += a;
			++Count;
		}
	}
	double Avg = Sum / double(Count);
	DebugNegative[0] = Sum;
	DebugNegative[1] = double(Count);
	DebugNegative[2] = Avg;
}

template<class T>
inline T AbsMax(const TVector<T>& Array)
{
	T Max = -std::numeric_limits<T>::infinity();
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
	GetDebugInfo(NegativeDivergence);
	float MaxDiv = AbsMax(NegativeDivergence);
	if (MaxDiv < PressureTolerance)
	{
		_LOG(Log, "Div (%f) is small enough, return 0 pressure.\n", MaxDiv);
		return;
	}

	BuildMatrixCoefficients(Parameter);
	GetDebugInfo(A);

	CalcPreconditioner(Parameter);
	GetDebugInfo(Precon);

	SolvePressure();
	GetDebugInfo(PressureResult);

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

	FFluidGrid3<float>* LiquidSdf = Parameter.LiquidSDF;
	TI_ASSERT(LiquidSdf->GetDimension() == Parameter.Pressure->GetDimension());
	// Collect 
	for (int32 Index = 0; Index < LiquidSdf->GetTotalCells(); Index++)
	{
		int32 BoundaryInfo = LiquidSdf->GetBoudaryTypeInfo(Index);
		if (BoundaryInfo == Boundary_None && LiquidSdf->Cell(Index) < 0.f)
		{
			vector3di GridIndex = LiquidSdf->ArrayIndexToGridIndex(Index);
			PressureGrids.push_back(GridIndex);
		}
	}

	GridsMap.clear();
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
	const FFluidGrid3<float>& WeightU = *Parameter.UW;
	const FFluidGrid3<float>& WeightV = *Parameter.VW;
	const FFluidGrid3<float>& WeightW = *Parameter.WW;
	vector3df InvCellSize = vector3df(1.f) / Parameter.CellSize;
		
	NegativeDivergence.clear();
	NegativeDivergence.resize(PressureGrids.size());

	for (uint32 Index = 0; Index < (uint32)PressureGrids.size(); Index++)
	{
		const vector3di& G = PressureGrids[Index];

		float ULW = WeightU.Cell(G.X, G.Y, G.Z);
		float URW = WeightU.Cell(G.X + 1, G.Y, G.Z);
		float VBW = WeightV.Cell(G.X, G.Y, G.Z);
		float VFW = WeightV.Cell(G.X, G.Y + 1, G.Z);
		float WDW = WeightW.Cell(G.X, G.Y, G.Z);
		float WUW = WeightW.Cell(G.X, G.Y, G.Z + 1);

		float UL0 = U.Cell(G.X, G.Y, G.Z);
		float UR0 = U.Cell(G.X + 1, G.Y, G.Z);
		float VB0 = V.Cell(G.X, G.Y, G.Z);
		float VF0 = V.Cell(G.X, G.Y + 1, G.Z);
		float WD0 = W.Cell(G.X, G.Y, G.Z);
		float WU0 = W.Cell(G.X, G.Y, G.Z + 1);

		float UL = ULW * UL0;
		float UR = URW * UR0;
		float VB = VBW * VB0;
		float VF = VFW * VF0;
		float WD = WDW * WD0;
		float WU = WUW * WU0;

		float Div = (UR - UL) * InvCellSize.X + (VF - VB) * InvCellSize.Y + (WU - WD) * InvCellSize.Z;
		// FLIPDebug
		if (TMath::Abs(Div)>0.0001f)
		{
			int32 aaaa = 0;
		}
		NegativeDivergence[Index] = -Div;
	}
}

void FPCGSolver::BuildMatrixCoefficients(const PCGSolverParameters& Parameter)
{
	A.clear();
	A.resize(PressureGrids.size());

	const FFluidGrid3<float>& WeightU = *Parameter.UW;
	const FFluidGrid3<float>& WeightV = *Parameter.VW;
	const FFluidGrid3<float>& WeightW = *Parameter.WW;
	const FFluidGrid3<float>& LiquidSDF = *Parameter.LiquidSDF;

	const float MinFrac = 0.01f;
	vector3df Scale = vector3df(Parameter.DeltaTime) / (Parameter.CellSize * Parameter.CellSize);
	float Term;
	for (uint32 Index = 0; Index < (uint32)PressureGrids.size(); Index++)
	{
		const vector3di& G = PressureGrids[Index];

		// Right
		Term = WeightU.Cell(G.X + 1, G.Y, G.Z) * Scale.X;
		float SdfRight = LiquidSDF.Cell(G.X + 1, G.Y, G.Z);
		if (SdfRight < 0)
		{
			A[Index].Diag += Term;
			A[Index].PlusI -= Term;
		}
		else
		{
			float Theta = TMath::Max(GetFaceWeight2U(LiquidSDF, G.X + 1, G.Y, G.Z), MinFrac);
			A[Index].Diag += Term / Theta;
		}

		// Left
		Term = WeightU.Cell(G) * Scale.X;
		float SdfLeft = LiquidSDF.Cell(G.X - 1, G.Y, G.Z);
		if (SdfLeft < 0)
		{
			A[Index].Diag += Term;
		}
		else
		{
			float Theta = TMath::Max(GetFaceWeight2U(LiquidSDF, G.X, G.Y, G.Z), MinFrac);
			A[Index].Diag += Term / Theta;
		}

		// Front
		Term = WeightV.Cell(G.X, G.Y + 1, G.Z) * Scale.Y;
		float SdfFront = LiquidSDF.Cell(G.X, G.Y + 1, G.Z);
		if (SdfFront < 0)
		{
			A[Index].Diag += Term;
			A[Index].PlusJ -= Term;
		}
		else
		{
			float Theta = TMath::Max(GetFaceWeight2V(LiquidSDF, G.X, G.Y + 1, G.Z), MinFrac);
			A[Index].Diag += Term / Theta;
		}

		// Back
		Term = WeightV.Cell(G) * Scale.Y;
		float SdfBack = LiquidSDF.Cell(G.X, G.Y - 1, G.Z);
		if (SdfBack < 0)
		{
			A[Index].Diag += Term;
		}
		else
		{
			float Theta = TMath::Max(GetFaceWeight2V(LiquidSDF, G.X, G.Y, G.Z), MinFrac);
			A[Index].Diag += Term / Theta;
		}

		// Up
		Term = WeightW.Cell(G.X, G.Y, G.Z + 1) * Scale.Z;
		float SdfUp = LiquidSDF.Cell(G.X, G.Y, G.Z + 1);
		if (SdfUp < 0)
		{
			A[Index].Diag += Term;
			A[Index].PlusK -= Term;
		}
		else
		{
			float Theta = TMath::Max(GetFaceWeight2W(LiquidSDF, G.X, G.Y, G.Z + 1), MinFrac);
			A[Index].Diag += Term / Theta;
		}

		// Down
		Term = WeightW.Cell(G) * Scale.Z;
		float SdfDown = LiquidSDF.Cell(G.X, G.Y, G.Z - 1);
		if (SdfDown < 0)
		{
			A[Index].Diag += Term;
		}
		else
		{
			float Theta = TMath::Max(GetFaceWeight2W(LiquidSDF, G.X, G.Y, G.Z), MinFrac);
			A[Index].Diag += Term / Theta;
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

		double Diag = A[Index].Diag;

		double PlusI_I = ItI != GridsMap.end() ? A[ItI->second].PlusI : 0.f;
		double PlusI_J = ItJ != GridsMap.end() ? A[ItJ->second].PlusI : 0.f;
		double PlusI_K = ItK != GridsMap.end() ? A[ItK->second].PlusI : 0.f;

		double PlusJ_I = ItI != GridsMap.end() ? A[ItI->second].PlusJ : 0.f;
		double PlusJ_J = ItJ != GridsMap.end() ? A[ItJ->second].PlusJ : 0.f;
		double PlusJ_K = ItK != GridsMap.end() ? A[ItK->second].PlusJ : 0.f;

		double PlusK_I = ItI != GridsMap.end() ? A[ItI->second].PlusK : 0.f;
		double PlusK_J = ItJ != GridsMap.end() ? A[ItJ->second].PlusK : 0.f;
		double PlusK_K = ItK != GridsMap.end() ? A[ItK->second].PlusK : 0.f;

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
	GetDebugNegative(Residual);

	TVector<double> Auxillary;
	Auxillary.resize(PressureGrids.size());
	ApplyPreconditioner(A, Precon, Residual, Auxillary);
	GetDebugInfo(Auxillary);
	GetDebugNegative(Precon);
	GetDebugNegative(Auxillary);

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
		ApplyMatrix(A, Search, Auxillary);
		GetDebugInfo(Auxillary);
		Alpha = Sigma / DotVector(Auxillary, Search);
		AddScaledVector(PressureResult, Search, Alpha);
		GetDebugInfo(PressureResult);
		GetDebugNegative(PressureResult);
		AddScaledVector(Residual, Auxillary, -Alpha);
		GetDebugInfo(Residual);

		double MaxCoeff = AbsMax(Residual);
		if (MaxCoeff < PressureTolerance)
		{
			_LOG(Log, "PCG Iter = %d, Error = %f.\n", Iter, MaxCoeff);
			return;
		}

		ApplyPreconditioner(A, Precon, Residual, Auxillary);
		GetDebugInfo(Auxillary);
		SigmaNew = DotVector(Auxillary, Residual);
		Beta = SigmaNew / Sigma;

		AddScaledVectors(Auxillary, 1.0, Search, Beta, Search);
		GetDebugInfo(Search);
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
			PreconI = PC[ItI->second];
			Q_I = Q[ItI->second];
		}

		double PlusJ_J = 0.0;
		double PreconJ = 0.0;
		double Q_J = 0.0;
		if (ItJ != GridsMap.end())
		{
			PlusJ_J = A[ItJ->second].PlusJ;
			PreconJ = PC[ItJ->second];
			Q_J = Q[ItJ->second];
		}

		double PlusK_K = 0.0;
		double PreconK = 0.0;
		double Q_K = 0.0;
		if (ItK != GridsMap.end())
		{
			PlusK_K = A[ItK->second].PlusK;
			PreconK = PC[ItK->second];
			Q_K = Q[ItK->second];
		}

		double T =
			double(Residual[Index]) -
			PlusI_I * PreconI * Q_I -
			PlusJ_J * PreconJ * Q_J -
			PlusK_K * PreconK * Q_K;

		T = T * PC[Index];
		Q[Index] = T;
	}

	// Solve transpose(A)*z = q
	for (int32 Index = (int32)PressureGrids.size() - 1; Index >= 0; Index--)
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
	TI_ASSERT(A.size() == X.size() && A.size() == Result.size());
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

void FPCGSolver::AddScaledVectors(const TVector<double>& V1, double S1, const TVector<double>& V2, double S2, TVector<double>& Result)
{
	TI_ASSERT(V1.size() == V2.size() && V1.size() == Result.size());

	for (uint32 i = 0; i < (uint32)V1.size(); i++)
	{
		Result[i] = V1[i] * S1 + V2[i] * S2;
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