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
const bool FPCGSolver::UseIPP = !false;
const int32 FPCGSolver::MaxPCGIterations = 200;

static pcg_float DebugFloat[3];

void GetDebugInfo(const TVector<FMatrixCell>& A)
{
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
}

//inline void GetDebugInfo(const TVector<float>& A)
//{
//#ifdef TIX_DEBUG
//	float Min = std::numeric_limits<float>::infinity();
//	float Max = -std::numeric_limits<float>::infinity();
//
//	float Sum = 0;
//	for (const auto& a : A)
//	{
//		Sum += a;
//		if (a < Min)
//			Min = a;
//		if (a > Max)
//			Max = a;
//	}
//	DebugFloat[0] = Min;
//	DebugFloat[1] = Max;
//	DebugFloat[2] = Sum / (float)(A.size());
//#endif
//}
inline void GetDebugInfo(const TVector<pcg_float>& A)
{
	pcg_float Min = std::numeric_limits<pcg_float>::infinity();
	pcg_float Max = -std::numeric_limits<pcg_float>::infinity();

	pcg_float Sum = 0;
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
	DebugFloat[2] = Sum / (pcg_float)(A.size());
}


pcg_float DebugNegative[3];
inline void GetDebugNegative(const TVector<pcg_float>& A)
{
	int Count = 0;
	pcg_float Sum = 0.f;
	for (const auto& a : A)
	{
		if (a < 0)
		{
			Sum += a;
			++Count;
		}
	}
	pcg_float Avg = Sum / pcg_float(Count);
	DebugNegative[0] = Sum;
	DebugNegative[1] = pcg_float(Count);
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
	pcg_float MaxDiv = AbsMax(NegativeDivergence);
	if (MaxDiv < PressureTolerance)
	{
		_LOG(Log, "Div (%f) is small enough, return 0 pressure.\n", MaxDiv);
		return;
	}

	BuildMatrix(Parameter);
	GetDebugInfo(A);
	if (0)
		OutputMatrix(*Parameter.Pressure);

	if (!UseIPP)
	{
		CalcPreconditionerMIC(Parameter);
		GetDebugInfo(Precon);
	}

	SolvePressure();
	GetDebugInfo(PressureResult);
	_LOG(Log, "Avg pressure %f.\n", DebugFloat[2]);

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

void FPCGSolver::BuildMatrix(const PCGSolverParameters& Parameter)
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

struct RowElement
{
	float Value;
	uint32 RowIndex;
	vector3di GridIndex;
};

void OutputRow(TStringStream& SS, TStringStream& SSPos, const TVector<RowElement>& Row, uint32 RowLength)
{
	uint32 i = 0;
	// write empty until diag
	for ( ; i < Row[0].RowIndex; i++)
	{
		SS << ",";
		SSPos << ",";
	}
	// write diag
	SS << Row[0].Value << ",";
	SSPos << "Diag(" << Row[0].GridIndex.X << "," << Row[0].GridIndex.Y << "," << Row[0].GridIndex.Z << "),";
	i++;
	// write Right;
	SS << Row[1].Value << ",";
	SSPos << "pI(" << Row[1].GridIndex.X << "," << Row[1].GridIndex.Y << "," << Row[1].GridIndex.Z << "),";
	i++;
	// write empty until front
	for (; i < Row[2].RowIndex; i++)
	{
		SS << ",";
		SSPos << ",";
	}
	// write front
	SS << Row[2].Value << ", ";
	SSPos << "pJ(" << Row[2].GridIndex.X << "," << Row[2].GridIndex.Y << "," << Row[2].GridIndex.Z << "),";
	i++;
	// write empty until top
	for (; i < Row[3].RowIndex; i++)
	{
		SS << ",";
		SSPos << ",";
	}
	// write top
	SS << Row[3].Value << ", ";
	SSPos << "pK(" << Row[3].GridIndex.X << "," << Row[3].GridIndex.Y << "," << Row[3].GridIndex.Z << "),";
	i++;
	// write empty until row end
	for (; i < RowLength; i++)
	{
		if (i < RowLength - 1)
		{
			SS << ",";
			SSPos << ",";
		}
		else
		{
			SS << "\n";
			SSPos << "\n";
		}
	}
}

void FPCGSolver::OutputMatrix(const FFluidGrid3<float>& PressureGrid)
{
	TStringStream MatrixSS;
	TStringStream MatrixPos;
	TVector<RowElement> Row;
	for (int32 i = 0; i < PressureGrid.GetTotalCells(); i++)
	{
		vector3di G = PressureGrid.ArrayIndexToGridIndex(i);

		Row.clear();

		FGridsMap::iterator It = GridsMap.find(G);
		if (It != GridsMap.end())
		{
			// has pressure equation
			vector3di GR = vector3di(G.X + 1, G.Y, G.Z);
			vector3di GF = vector3di(G.X, G.Y + 1, G.Z);
			vector3di GT = vector3di(G.X, G.Y, G.Z + 1);
			uint32 IndexR = PressureGrid.GridIndexToArrayIndex(GR.X, GR.Y, GR.Z);
			uint32 IndexF = PressureGrid.GridIndexToArrayIndex(GF.X, GF.Y, GF.Z);
			uint32 IndexT = PressureGrid.GridIndexToArrayIndex(GT.X, GT.Y, GT.Z);

			Row.push_back({ A[It->second].Diag, (uint32)i, G });
			Row.push_back({ A[It->second].PlusI, IndexR, GR });
			Row.push_back({ A[It->second].PlusJ, IndexF, GF });
			Row.push_back({ A[It->second].PlusK, IndexT, GT });

			OutputRow(MatrixSS, MatrixPos, Row, PressureGrid.GetTotalCells());
		}
		else
		{
			// do not have pressure equation
		}
	}

	TString FN = "matrix.csv";
	TFile F;
	if (F.Open(FN, EFA_CREATEWRITE))
	{
		F.Write(MatrixSS.str().c_str(), (int32)MatrixSS.str().length());
		F.Close();
	}
	if (F.Open("matrix1.csv", EFA_CREATEWRITE))
	{
		F.Write(MatrixPos.str().c_str(), (int32)MatrixPos.str().length());
		F.Close();
	}
}

void FPCGSolver::CalcPreconditionerMIC(const PCGSolverParameters& Parameter)
{
	Precon.clear();
	Precon.resize(PressureGrids.size());

	const pcg_float Tau = 0.97f;	// Tuning constant
	const pcg_float Sigma = 0.25f;	// Safety constant

	for (uint32 Index = 0; Index < (uint32)PressureGrids.size(); Index++)
	{
		const vector3di& GridIndex = PressureGrids[Index];

		FGridsMap::iterator ItI = GridsMap.find(vector3di(GridIndex.X - 1, GridIndex.Y, GridIndex.Z));
		FGridsMap::iterator ItJ = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y - 1, GridIndex.Z));
		FGridsMap::iterator ItK = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y, GridIndex.Z - 1));

		pcg_float Diag = A[Index].Diag;

		pcg_float PlusI_I = ItI != GridsMap.end() ? A[ItI->second].PlusI : 0.f;
		pcg_float PlusI_J = ItJ != GridsMap.end() ? A[ItJ->second].PlusI : 0.f;
		pcg_float PlusI_K = ItK != GridsMap.end() ? A[ItK->second].PlusI : 0.f;

		pcg_float PlusJ_I = ItI != GridsMap.end() ? A[ItI->second].PlusJ : 0.f;
		pcg_float PlusJ_J = ItJ != GridsMap.end() ? A[ItJ->second].PlusJ : 0.f;
		pcg_float PlusJ_K = ItK != GridsMap.end() ? A[ItK->second].PlusJ : 0.f;

		pcg_float PlusK_I = ItI != GridsMap.end() ? A[ItI->second].PlusK : 0.f;
		pcg_float PlusK_J = ItJ != GridsMap.end() ? A[ItJ->second].PlusK : 0.f;
		pcg_float PlusK_K = ItK != GridsMap.end() ? A[ItK->second].PlusK : 0.f;

		pcg_float PreconI = ItI != GridsMap.end() ? Precon[ItI->second] : 0.f;
		pcg_float PreconJ = ItJ != GridsMap.end() ? Precon[ItJ->second] : 0.f;
		pcg_float PreconK = ItK != GridsMap.end() ? Precon[ItK->second] : 0.f;

		pcg_float V1 = PlusI_I * PreconI;
		pcg_float V2 = PlusJ_J * PreconJ;
		pcg_float V3 = PlusK_K * PreconK;
		pcg_float V4 = PreconI * PreconI;
		pcg_float V5 = PreconJ * PreconJ;
		pcg_float V6 = PreconK * PreconK;

		pcg_float e =
			Diag - V1 * V1 - V2 * V2 - V3 * V3 - 
			Tau * (PlusI_I * (PlusJ_I + PlusK_I) * V4 +
				   PlusJ_J * (PlusI_J + PlusK_J) * V5 +
				   PlusK_K * (PlusI_K + PlusJ_K) * V6
				  );

		if (e < Sigma * Diag)
			e = Diag;

		if (fabs(e) > 10e-9)
		{
			Precon[Index] = 1.0f / sqrt(e);
		}
	}
}

void FPCGSolver::SolvePressure()
{
	TVector<pcg_float> Residual;
	Residual.resize(NegativeDivergence.size());
	for (uint32 Index = 0; Index < (uint32)NegativeDivergence.size(); Index++)
	{
		Residual[Index] = NegativeDivergence[Index];
	}
	GetDebugNegative(Residual);

	TVector<pcg_float> Auxillary;
	Auxillary.resize(PressureGrids.size());
	if (UseIPP)
		ApplyPreconditionerIPP(A, Residual, Auxillary);
	else
		ApplyPreconditionerMIC(A, Precon, Residual, Auxillary);
	GetDebugInfo(Auxillary);
	GetDebugNegative(Precon);
	GetDebugNegative(Auxillary);

	TVector<pcg_float> Search;
	Search = Auxillary;

	pcg_float Alpha = 0.0;
	pcg_float Beta = 0.0;
	pcg_float Sigma = DotVector(Auxillary, Residual);
	pcg_float SigmaNew = 0.0;
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

		pcg_float MaxCoeff = AbsMax(Residual);
		if (MaxCoeff < PressureTolerance)
		{
			_LOG(Log, "PCG Iter = %d, Error = %f.\n", Iter, MaxCoeff);
			return;
		}

		if (UseIPP)
			ApplyPreconditionerIPP(A, Residual, Auxillary);
		else
			ApplyPreconditionerMIC(A, Precon, Residual, Auxillary);
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

void FPCGSolver::ApplyPreconditionerMIC(const TVector<FMatrixCell>& A, const TVector<pcg_float>& PC, const TVector<pcg_float>& Residual, TVector<pcg_float>& Auxillary)
{
	// Solve A*q = residual
	TVector<pcg_float> Q;
	Q.resize(PressureGrids.size());
	for (uint32 Index = 0; Index < (uint32)PressureGrids.size(); Index++)
	{
		const vector3di& GridIndex = PressureGrids[Index];

		FGridsMap::iterator ItI = GridsMap.find(vector3di(GridIndex.X - 1, GridIndex.Y, GridIndex.Z));
		FGridsMap::iterator ItJ = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y - 1, GridIndex.Z));
		FGridsMap::iterator ItK = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y, GridIndex.Z - 1));

		pcg_float PlusI_I = 0.0;
		pcg_float PreconI = 0.0;
		pcg_float Q_I = 0.0;
		if (ItI != GridsMap.end())
		{
			PlusI_I = A[ItI->second].PlusI;
			PreconI = PC[ItI->second];
			Q_I = Q[ItI->second];
		}

		pcg_float PlusJ_J = 0.0;
		pcg_float PreconJ = 0.0;
		pcg_float Q_J = 0.0;
		if (ItJ != GridsMap.end())
		{
			PlusJ_J = A[ItJ->second].PlusJ;
			PreconJ = PC[ItJ->second];
			Q_J = Q[ItJ->second];
		}

		pcg_float PlusK_K = 0.0;
		pcg_float PreconK = 0.0;
		pcg_float Q_K = 0.0;
		if (ItK != GridsMap.end())
		{
			PlusK_K = A[ItK->second].PlusK;
			PreconK = PC[ItK->second];
			Q_K = Q[ItK->second];
		}

		pcg_float T =
			pcg_float(Residual[Index]) -
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

		pcg_float VectI = ItI != GridsMap.end() ? Auxillary[ItI->second] : 0.0f;
		pcg_float VectJ = ItJ != GridsMap.end() ? Auxillary[ItJ->second] : 0.0f;
		pcg_float VectK = ItK != GridsMap.end() ? Auxillary[ItK->second] : 0.0f;

		pcg_float PlusI = A[Index].PlusI;
		pcg_float PlusJ = A[Index].PlusJ;
		pcg_float PlusK = A[Index].PlusK;

		pcg_float PreconValue = PC[Index];
		pcg_float T = Q[Index] -
			PlusI * PreconValue * VectI -
			PlusJ * PreconValue * VectJ -
			PlusK * PreconValue * VectK;

		T = T * PreconValue;
		Auxillary[Index] = T;
	}
}

void FPCGSolver::ApplyPreconditionerIPP(const TVector<FMatrixCell>& A, const TVector<pcg_float>& Residual, TVector<pcg_float>& Auxillary)
{
	// calc z[i] = r[i] - r * L[i, :] * 1/d
	TVector<pcg_float> Z;
	Z.resize(PressureGrids.size());
	for (uint32 Index = 0; Index < (uint32)PressureGrids.size(); Index++)
	{
		const vector3di& GridIndex = PressureGrids[Index];

		FGridsMap::iterator ItI = GridsMap.find(vector3di(GridIndex.X - 1, GridIndex.Y, GridIndex.Z));
		FGridsMap::iterator ItJ = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y - 1, GridIndex.Z));
		FGridsMap::iterator ItK = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y, GridIndex.Z - 1));

		float InvDi = 1.f / A[Index].Diag;
		float Ri = Residual[Index];
		pcg_float PlusI = A[Index].PlusI;
		pcg_float PlusJ = A[Index].PlusJ;
		pcg_float PlusK = A[Index].PlusK;

		//pcg_float Sum = 0.f;
		//if (ItI != GridsMap.end())
		//{
		//	Sum += PlusI * Residual[ItI->second];
		//}
		//if (ItJ != GridsMap.end())
		//{
		//	Sum += PlusJ * Residual[ItJ->second];
		//}
		//if (ItK != GridsMap.end())
		//{
		//	Sum += PlusK * Residual[ItK->second];
		//}
		//Z[Index] = Ri - InvDi * Sum;
		pcg_float Sum = 0.f;
		if (ItI != GridsMap.end() && A[ItI->second].Diag > 0.f)
		{
			Sum += 1.f / A[ItI->second].Diag * PlusI * Residual[ItI->second];
		}
		if (ItJ != GridsMap.end() && A[ItJ->second].Diag)
		{
			Sum += 1.f / A[ItJ->second].Diag * PlusJ * Residual[ItJ->second];
		}
		if (ItK != GridsMap.end() && A[ItK->second].Diag)
		{
			Sum += 1.f / A[ItK->second].Diag * PlusK * Residual[ItK->second];
		}
		Z[Index] = Ri - Sum;
	}

	// calc z[i] = z[i] - z * L[i, :] * 1/d
	TFill(Auxillary.begin(), Auxillary.end(), pcg_float(0));
	for (int32 Index = (int32)PressureGrids.size() - 1; Index >= 0; Index--)
	{
		const vector3di& GridIndex = PressureGrids[Index];

		FGridsMap::iterator ItI = GridsMap.find(vector3di(GridIndex.X + 1, GridIndex.Y, GridIndex.Z));
		FGridsMap::iterator ItJ = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y + 1, GridIndex.Z));
		FGridsMap::iterator ItK = GridsMap.find(vector3di(GridIndex.X, GridIndex.Y, GridIndex.Z + 1));

		pcg_float PlusI = A[Index].PlusI;
		pcg_float PlusJ = A[Index].PlusJ;
		pcg_float PlusK = A[Index].PlusK;

		//pcg_float Sum = 0.f;
		//if (ItI != GridsMap.end())
		//{
		//	Sum += 1.f / A[ItI->second].Diag * PlusI * Z[ItI->second];
		//}
		//if (ItJ != GridsMap.end())
		//{
		//	Sum += 1.f / A[ItJ->second].Diag * PlusJ * Z[ItJ->second];
		//}
		//if (ItK != GridsMap.end())
		//{
		//	Sum += 1.f / A[ItK->second].Diag * PlusK * Z[ItK->second];
		//}
		//Auxillary[Index] = Z[Index] - Sum;

		TI_ASSERT(A[Index].Diag > 0.f);
		pcg_float InvDi = 1.f / A[Index].Diag;
		pcg_float Sum = 0.f;
		if (ItI != GridsMap.end())
		{
			Sum += PlusI * Z[ItI->second];
		}
		if (ItJ != GridsMap.end())
		{
			Sum += PlusJ * Z[ItJ->second];
		}
		if (ItK != GridsMap.end())
		{
			Sum += PlusK * Z[ItK->second];
		}
		Auxillary[Index] = Z[Index] - InvDi * Sum;
	}
}

void FPCGSolver::ApplyMatrix(const TVector<FMatrixCell>& A, const TVector<pcg_float>& X, TVector<pcg_float>& Result)
{
	TI_ASSERT(A.size() == X.size() && A.size() == Result.size());
	for (uint32 Index = 0; Index < (uint32)PressureGrids.size(); Index++)
	{
		const vector3di& GridIndex = PressureGrids[Index];

		FGridsMap::iterator It;
		pcg_float Val = 0.0;
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

void FPCGSolver::AddScaledVector(TVector<pcg_float>& V1, const TVector<pcg_float>& V2, pcg_float Scale)
{
	TI_ASSERT(V1.size() == V2.size());

	for (uint32 i = 0; i < (uint32)V1.size(); i++)
	{
		V1[i] += V2[i] * Scale;
	}
}

void FPCGSolver::AddScaledVectors(const TVector<pcg_float>& V1, pcg_float S1, const TVector<pcg_float>& V2, pcg_float S2, TVector<pcg_float>& Result)
{
	TI_ASSERT(V1.size() == V2.size() && V1.size() == Result.size());

	for (uint32 i = 0; i < (uint32)V1.size(); i++)
	{
		Result[i] = V1[i] * S1 + V2[i] * S2;
	}
}

pcg_float FPCGSolver::DotVector(const TVector<pcg_float>& V1, const TVector<pcg_float>& V2)
{
	TI_ASSERT(V1.size() == V2.size());

	pcg_float Sum = 0.0;
	for (uint32 i = 0; i < (uint32)V1.size(); i++)
	{
		Sum += V1[i] * V2[i];
	}
	return Sum;
}