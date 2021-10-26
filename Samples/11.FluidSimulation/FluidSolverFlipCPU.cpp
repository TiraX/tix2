/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSolverFlipCPU.h"
#include "FluidSimRenderer.h"
#include "LevelsetUtils.h"


static float DebugFloat[3];
inline void GetDebugInfo(const FFluidGrid3<float>& A)
{
	float Min = std::numeric_limits<float>::infinity();
	float Max = -std::numeric_limits<float>::infinity();

	float Sum = 0;
	for (int32 i = 0; i < A.GetTotalCells(); i++)
	{
		float a = A.Cell(i);
		Sum += a;
		if (a < Min)
			Min = a;
		if (a > Max)
			Max = a;
	}
	DebugFloat[0] = Min;
	DebugFloat[1] = Max;
	DebugFloat[2] = Sum / (float)(A.GetTotalCells());
}

#define DO_PARALLEL (0)

const float eps = 1e-9f;
const int32 UNKNOWN = 0x00;
const int32 WAITING = 0x01;
const int32 KNOWN = 0x02;
const int32 DONE = 0x03;


template < class T>
inline vector3d<T> ClampVector3d(const vector3d<T>& V, const vector3d<T>& Min, const vector3d<T>& Max)
{
	vector3d<T> Result;
	Result.X = TMath::Clamp(V.X, Min.X, Max.X);
	Result.Y = TMath::Clamp(V.Y, Min.Y, Max.Y);
	Result.Z = TMath::Clamp(V.Z, Min.Z, Max.Z);
	return Result;
}

template <class T>
inline vector3d<T> MinVector3d(const vector3d<T>& A, const vector3d<T>& B)
{
	vector3d<T> Result;
	Result.X = TMath::Min(A.X, B.X);
	Result.Y = TMath::Min(A.Y, B.Y);
	Result.Z = TMath::Min(A.Z, B.Z);
	return Result;
}

template <class T>
inline vector3d<T> MaxVector3d(const vector3d<T>& A, const vector3d<T>& B)
{
	vector3d<T> Result;
	Result.X = TMath::Max(A.X, B.X);
	Result.Y = TMath::Max(A.Y, B.Y);
	Result.Z = TMath::Max(A.Z, B.Z);
	return Result;
}

inline vector3di Floor(const vector3df& x)
{
	vector3di i;
	i.X = TMath::Floor(x.X);
	i.Y = TMath::Floor(x.Y);
	i.Z = TMath::Floor(x.Z);
	return i;
}

FFluidSolverFlipCPU::FFluidSolverFlipCPU()
	: ParticleRadius(0.f)
	, SolidSDF(nullptr)
{
	SubStep = 1;
}

FFluidSolverFlipCPU::~FFluidSolverFlipCPU()
{
	if (SolidSDF != nullptr)
	{
		ti_delete SolidSDF;
	}
}

void FFluidSolverFlipCPU::CreateParticles(
	const aabbox3df& InParticleBox,
	float InParticleSeperation,
	float InParticleMass)
{
	Particles.CreateParticlesInBox(InParticleBox, InParticleSeperation, InParticleMass);
	Flag |= DirtyParticles;
	Flag |= DirtyParams;
}

void FFluidSolverFlipCPU::CreateGrid(const vector3di& Dim)
{
	CellSize.X = BoundaryBox.getExtent().X / Dim.X;
	CellSize.Y = BoundaryBox.getExtent().Y / Dim.Y;
	CellSize.Z = BoundaryBox.getExtent().Z / Dim.Z;

	const float MaxSize = TMath::Max3(CellSize.X, CellSize.Y, CellSize.Z);
	ParticleRadius = MaxSize * 1.01f * sqrt(3.f) / 2.f;

	InvCellSize = vector3df(1.f) / CellSize;
	
	Dimension = Dim;
	VelField[0].Create(vector3di(Dim.X + 1, Dim.Y, Dim.Z));
	VelField[1].Create(vector3di(Dim.X, Dim.Y + 1, Dim.Z));
	VelField[2].Create(vector3di(Dim.X, Dim.Y, Dim.Z + 1));
	//Marker.Create(Dim);
	Divergence.Create(Dim);
	Pressure.Create(Dim);
	VelFieldDelta[0].Create(VelField[0].GetDimension());
	VelFieldDelta[1].Create(VelField[1].GetDimension());
	VelFieldDelta[2].Create(VelField[2].GetDimension());
	IsValidVelocity[0].Create(VelField[0].GetDimension());
	IsValidVelocity[1].Create(VelField[1].GetDimension());
	IsValidVelocity[2].Create(VelField[2].GetDimension());
	WeightGrid[0].Create(VelField[0].GetDimension());
	WeightGrid[1].Create(VelField[1].GetDimension());
	WeightGrid[2].Create(VelField[2].GetDimension());	
	LiquidSDF.Create(Dim);
}

void FFluidSolverFlipCPU::AddCollision(FGeometrySDF* InCollisionSDF)
{
	SolidSDF = InCollisionSDF;
}

int32 Counter = 0;
void FFluidSolverFlipCPU::Sim(FRHI * RHI, float Dt)
{
	TIMER_RECORDER("FlipSim");
	UpdateLiquidSDF();
	AdvectVelocityField();
	ExtrapolateVelocityField();
	MarkCells();
	BackupVelocity();
	CalcExternalForces(Dt);
	CalcVisicosity(Dt);
	ComputeWeights();
	SolvePressure(Dt);
	ApplyPressure(Dt);
	ExtrapolateVelocityField();
	ConstrainVelocityField();
	AdvectParticles(Dt);
}

void FFluidSolverFlipCPU::UpdateLiquidSDF()
{
	// Compute Signed Distance From Particles
	const float MaxDistance = TMath::Max3(CellSize.X, CellSize.Y, CellSize.Z) * 3.f;
	const float MaxDistanceH = MaxDistance * 0.5f;
	LiquidSDF.Fill(MaxDistance);

	const vector3df CellSizeH = CellSize * 0.5f;
	for (int32 Index = 0; Index < Particles.GetTotalParticles(); Index++)
	{
		const vector3df& P = Particles.GetParticlePosition(Index);
		vector3di IndexInGrid = Floor(P * InvCellSize);
		vector3di GridMin = vector3di(IndexInGrid.X - 1, IndexInGrid.Y - 1, IndexInGrid.Z - 1);
		vector3di GridMax = vector3di(IndexInGrid.X + 1, IndexInGrid.Y + 1, IndexInGrid.Z + 1);
		const vector3di& GridDim = Pressure.GetDimension();
		GridMin = MaxVector3d(GridMin, vector3di());
		GridMax = MinVector3d(GridMax, vector3di(GridDim.X - 1, GridDim.Y - 1, GridDim.Z - 1));

		for (int32 k = GridMin.Z; k <= GridMax.Z; k++)
		{
			for (int32 j = GridMin.Y; j <= GridMax.Y; j++)
			{
				for (int32 i = GridMin.X; i <= GridMax.X; i++)
				{
					vector3df CellCenter = vector3df(float(i), float(j), float(k)) * CellSize + CellSizeH;
					float Distance = (P - CellCenter).getLength() - ParticleRadius;
					if (Distance < LiquidSDF.Cell(i, j, k))
					{
						LiquidSDF.Cell(i, j, k) = Distance;
					}
				}
			}
		}
	}
	
	// Extrapolate Signed Distance Into Solids
	for (int32 Index = 0; Index < LiquidSDF.GetTotalCells(); Index++)
	{
		if (LiquidSDF.Cell(Index) < MaxDistanceH)
		{
			// Inside liquid
			vector3di G = LiquidSDF.ArrayIndexToGridIndex(Index);
			vector3df CellCenter = vector3df(float(G.X), float(G.Y), float(G.Z)) * CellSize + CellSizeH;
			if (SolidSDF->SampleSDFByPosition(CellCenter) < 0.f)
			{
				// Inside solid
				LiquidSDF.Cell(Index) = -MaxDistanceH;
			}
		}
	}
}

void FFluidSolverFlipCPU::AdvectVelocityField()
{
	// Mark fluid cells
	FFluidGrid3<int8> FluidCells;
	FluidCells.Create(Dimension);
	for (int32 Index = 0; Index < FluidCells.GetTotalCells(); Index++)
	{
		if (LiquidSDF.Cell(Index) < 0.f)
		{
			// Inside liquid
			FluidCells.Cell(Index) = 1;
		}
	}

	const vector3df HalfCellSize = CellSize * 0.5f;

	vector3df VelStartPos[3];
	VelStartPos[0] = vector3df(0.f, HalfCellSize.Y, HalfCellSize.Z);
	VelStartPos[1] = vector3df(HalfCellSize.X, 0.f, HalfCellSize.Z);
	VelStartPos[2] = vector3df(HalfCellSize.X, HalfCellSize.Y, 0.f);

	FFluidGrid3<float> VelInterpolate[3];
	VelInterpolate[0].Create(VelField[0].GetDimension());
	VelInterpolate[1].Create(VelField[1].GetDimension());
	VelInterpolate[2].Create(VelField[2].GetDimension());

	FFluidGrid3<float> Weights[3];
	Weights[0].Create(VelField[0].GetDimension());
	Weights[1].Create(VelField[1].GetDimension());
	Weights[2].Create(VelField[2].GetDimension());

	float r[3], rsq[3], coef1[3], coef2[3], coef3[3];
	for (int32 i = 0; i < 3; i++)
	{
		r[i] = CellSize[i];
		rsq[i] = r[i] * r[i];
		coef1[i] = (4.f / 9.f) * (1.f / (rsq[i] * rsq[i] * rsq[i]));
		coef2[i] = (17.f / 9.f) * (1.f / (rsq[i] * rsq[i]));
		coef3[i] = (22.f / 9.f) * (1.f / rsq[i]);
	}

	// Advect U,V,W
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0 ; Index < Particles.GetTotalParticles(); Index ++)
	{
		for (int32 VelIndex = 0; VelIndex < 3; VelIndex++)
		{
			const vector3df& Vel = Particles.GetParticleVelocity(Index);
			vector3df PosInGrid = Particles.GetParticlePosition(Index) - VelStartPos[VelIndex];
			vector3di IndexInGrid = Floor(PosInGrid * InvCellSize);
			vector3di GridMin = vector3di(IndexInGrid.X - 1, IndexInGrid.Y - 1, IndexInGrid.Z - 1);
			vector3di GridMax = vector3di(IndexInGrid.X + 1, IndexInGrid.Y + 1, IndexInGrid.Z + 1);
			const vector3di& GridDim = VelInterpolate[VelIndex].GetDimension();
			GridMin = MaxVector3d(GridMin, vector3di());
			GridMax = MinVector3d(GridMax, vector3di(GridDim.X - 1, GridDim.Y - 1, GridDim.Z - 1));

			for (int32 k = GridMin.Z; k <= GridMax.Z; k++)
			{
				for (int32 j = GridMin.Y; j <= GridMax.Y; j++)
				{
					for (int32 i = GridMin.X; i <= GridMax.X; i++)
					{
						vector3df GridPos = vector3df(float(i), float(j), float(k)) * CellSize;
						vector3df DisVector = GridPos - PosInGrid;
						float DisSQ = DisVector.dotProduct(DisVector);
						if (DisSQ < rsq[VelIndex])
						{
							float Weight =
								1.f - coef1[VelIndex] * DisSQ * DisSQ * DisSQ +
								coef2[VelIndex] * DisSQ * DisSQ -
								coef3[VelIndex] * DisSQ;
#if DO_PARALLEL
#pragma omp critical
#endif
							{
								VelInterpolate[VelIndex].Cell(i, j, k) += Vel[VelIndex] * Weight;
								Weights[VelIndex].Cell(i, j, k) += Weight;
							}
						}
					}
				}
			}
		}
	}

	const int8 kValid = 1;
	// Clear vel and weight to 0
	VelField[0].Clear();
	VelField[1].Clear();
	VelField[2].Clear();
	IsValidVelocity[0].Clear();
	IsValidVelocity[1].Clear();
	IsValidVelocity[2].Clear();
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[0].GetTotalCells(); Index++)
	{
		if (Weights[0].Cell(Index) > eps)
		{
			IsValidVelocity[0].Cell(Index) = 1;
			vector3di G = IsValidVelocity[0].ArrayIndexToGridIndex(Index);
			if (IsFaceBorderValueOfGridU(G.X, G.Y, G.Z, FluidCells, kValid))
				VelField[0].Cell(Index) = VelInterpolate[0].Cell(Index) / Weights[0].Cell(Index);
		}
	}

#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[1].GetTotalCells(); Index++)
	{
		if (Weights[1].Cell(Index) > eps)
		{
			IsValidVelocity[1].Cell(Index) = 1;
			vector3di G = IsValidVelocity[1].ArrayIndexToGridIndex(Index);
			if (IsFaceBorderValueOfGridV(G.X, G.Y, G.Z, FluidCells, kValid))
				VelField[1].Cell(Index) = VelInterpolate[1].Cell(Index) / Weights[1].Cell(Index);
		}
	}

#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[2].GetTotalCells(); Index++)
	{
		if (Weights[2].Cell(Index) > eps)
		{
			IsValidVelocity[2].Cell(Index) = 1;
			vector3di G = IsValidVelocity[2].ArrayIndexToGridIndex(Index);
			if (IsFaceBorderValueOfGridW(G.X, G.Y, G.Z, FluidCells, kValid))
				VelField[2].Cell(Index) = VelInterpolate[2].Cell(Index) / Weights[2].Cell(Index);
		}
	}
}

void TryToMarkCell(FFluidGrid3<int8>& Status, TVector<vector3di>& ExtrapolateCells, int32& Count, const vector3di& GridIndex)
{
	if (Status.Cell(GridIndex) == UNKNOWN)
	{
		ExtrapolateCells.push_back(GridIndex);
		Status.Cell(GridIndex) = WAITING;
		Count++;
	}
	else if (Status.Cell(GridIndex) == WAITING)
	{
		Count++;
	}
}

void FFluidSolverFlipCPU::ExtrapolateVelocityField()
{
	const int32 ExtraLayers = 7;

	FFluidGrid3<int8> Status[3];
	Status[0].Create(VelField[0].GetDimension());
	Status[1].Create(VelField[1].GetDimension());
	Status[2].Create(VelField[2].GetDimension());

#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[0].GetTotalCells(); Index++)
	{
		vector3di G = VelField[0].ArrayIndexToGridIndex(Index);
		Status[0].Cell(Index) = IsValidVelocity[0].Cell(Index) == 1 ? KNOWN : UNKNOWN;
		uint32 Info = VelField[0].GetBoudaryTypeInfo(Index);
		if (Status[0].Cell(Index) == UNKNOWN && Info != Boundary_None)
		{
			Status[0].Cell(Index) = DONE;
		}
	}
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[1].GetTotalCells(); Index++)
	{
		vector3di G = VelField[1].ArrayIndexToGridIndex(Index);
		Status[1].Cell(Index) = IsValidVelocity[1].Cell(Index) == 1 ? KNOWN : UNKNOWN;
		uint32 Info = VelField[1].GetBoudaryTypeInfo(Index);
		if (Status[1].Cell(Index) == UNKNOWN && Info != Boundary_None)
		{
			Status[1].Cell(Index) = DONE;
		}
	}
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[2].GetTotalCells(); Index++)
	{
		vector3di G = VelField[2].ArrayIndexToGridIndex(Index);
		Status[2].Cell(Index) = IsValidVelocity[2].Cell(Index) == 1 ? KNOWN : UNKNOWN;
		uint32 Info = VelField[2].GetBoudaryTypeInfo(Index);
		if (Status[2].Cell(Index) == UNKNOWN && Info != Boundary_None)
		{
			Status[2].Cell(Index) = DONE;
		}
	}

	TVector<vector3di> ExtrapolationCells;
	for (int32 VelIndex = 0; VelIndex < 3; VelIndex++)
	{
		for (int layers = 0; layers < ExtraLayers; layers++) 
		{
			ExtrapolationCells.clear();

#if DO_PARALLEL
#pragma omp parallel for
#endif
			for (int32 Index = 0; Index < VelField[VelIndex].GetTotalCells(); Index++)
			{
				vector3di GridIndex = VelField[VelIndex].ArrayIndexToGridIndex(Index);
				uint32 BoundaryInfo = VelField[VelIndex].GetBoudaryTypeInfo(Index);
				if (BoundaryInfo != Boundary_None || Status[VelIndex].Cell(Index) != KNOWN)
					continue;

				int32 Count = 0;
				TryToMarkCell(Status[VelIndex], ExtrapolationCells, Count, vector3di(GridIndex.X - 1, GridIndex.Y, GridIndex.Z));
				TryToMarkCell(Status[VelIndex], ExtrapolationCells, Count, vector3di(GridIndex.X + 1, GridIndex.Y, GridIndex.Z));
				TryToMarkCell(Status[VelIndex], ExtrapolationCells, Count, vector3di(GridIndex.X, GridIndex.Y - 1, GridIndex.Z));
				TryToMarkCell(Status[VelIndex], ExtrapolationCells, Count, vector3di(GridIndex.X, GridIndex.Y + 1, GridIndex.Z));
				TryToMarkCell(Status[VelIndex], ExtrapolationCells, Count, vector3di(GridIndex.X, GridIndex.Y, GridIndex.Z - 1));
				TryToMarkCell(Status[VelIndex], ExtrapolationCells, Count, vector3di(GridIndex.X, GridIndex.Y, GridIndex.Z + 1));

				if (Count == 0)
				{
					Status[VelIndex].Cell(Index) = DONE;
				}
			}

			vector3di G;
			for (size_t i = 0; i < ExtrapolationCells.size(); i++) 
			{
				G = ExtrapolationCells[i];

				float Sum = 0.f;
				int Count = 0;
				if (Status[VelIndex].Cell(G.X - 1, G.Y, G.Z) == KNOWN) { Sum += VelField[VelIndex].Cell(G.X - 1, G.Y, G.Z); Count++; }
				if (Status[VelIndex].Cell(G.X + 1, G.Y, G.Z) == KNOWN) { Sum += VelField[VelIndex].Cell(G.X + 1, G.Y, G.Z); Count++; }
				if (Status[VelIndex].Cell(G.X, G.Y - 1, G.Z) == KNOWN) { Sum += VelField[VelIndex].Cell(G.X, G.Y - 1, G.Z); Count++; }
				if (Status[VelIndex].Cell(G.X, G.Y + 1, G.Z) == KNOWN) { Sum += VelField[VelIndex].Cell(G.X, G.Y + 1, G.Z); Count++; }
				if (Status[VelIndex].Cell(G.X, G.Y, G.Z - 1) == KNOWN) { Sum += VelField[VelIndex].Cell(G.X, G.Y, G.Z - 1); Count++; }
				if (Status[VelIndex].Cell(G.X, G.Y, G.Z + 1) == KNOWN) { Sum += VelField[VelIndex].Cell(G.X, G.Y, G.Z + 1); Count++; }

				TI_ASSERT(Count != 0);
				VelField[VelIndex].Cell(G) = Sum / (float)Count;
				Status[VelIndex].Cell(G) = KNOWN;
			}
		}
	}
}

void FFluidSolverFlipCPU::MarkCells()
{
//	Marker.Clear();
//#if DO_PARALLEL
//#pragma omp parallel for
//#endif
//	for (int32 Index = 0; Index < Particles.GetTotalParticles(); Index++)
//	{
//		const vector3df& PosInGrid = Particles.GetParticlePosition(Index);
//		vector3di IndexInGrid = Floor(PosInGrid * InvCellSize);
//		Marker.Cell(IndexInGrid) = Marker_Fluid;
//	}
}

void FFluidSolverFlipCPU::BackupVelocity()
{
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[0].GetTotalCells(); Index++)
	{
		VelFieldDelta[0].Cell(Index) = VelField[0].Cell(Index);
	}
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[1].GetTotalCells(); Index++)
	{
		VelFieldDelta[1].Cell(Index) = VelField[1].Cell(Index);
	}
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[2].GetTotalCells(); Index++)
	{
		VelFieldDelta[2].Cell(Index) = VelField[2].Cell(Index);
	}
}

void FFluidSolverFlipCPU::CalcExternalForces(float Dt)
{
	// Mark fluid cells
	FFluidGrid3<int8> FluidCells;
	FluidCells.Create(Dimension);
	for (int32 Index = 0; Index < FluidCells.GetTotalCells(); Index++)
	{
		if (LiquidSDF.Cell(Index) < 0.f)
		{
			// Inside liquid
			FluidCells.Cell(Index) = 1;
		}
	}

	const vector3df Gravity = vector3df(0.f, 0.f, -9.81f);
	const vector3df DV = Gravity * Dt;
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[2].GetTotalCells(); Index++)
	{
		vector3di GridIndex = VelField[2].ArrayIndexToGridIndex(Index);
		if (IsFaceBorderValueOfGridW(GridIndex.X, GridIndex.Y, GridIndex.Z, FluidCells, int8(1)))
		{
			VelField[2].Cell(Index) += DV.Z;
		}
	}
}

void FFluidSolverFlipCPU::CalcVisicosity(float Dt)
{

}

void FFluidSolverFlipCPU::ComputeWeights()
{
	const vector3df CellSizeH = CellSize * 0.5f;
	vector3df VelocityStart[3] =
	{
		vector3df(),
		vector3df(),
		vector3df()
		//vector3df(0.f, CellSizeH.Y, CellSizeH.Z),
		//vector3df(CellSizeH.X, 0.f, CellSizeH.Z),
		//vector3df(CellSizeH.X, CellSizeH.Y, 0.f)
	};
	//Compute finite-volume type face area weight for each velocity sample.
    //Compute face area fractions (using marching squares cases).
	for (int32 Index = 0; Index < VelField[0].GetTotalCells(); Index++)
	{
		vector3di G = VelField[0].ArrayIndexToGridIndex(Index);
		vector3df Pos = vector3df(float(G.X), float(G.Y), float(G.Z)) * CellSize + VelocityStart[0];
		float W = 1.f - GetFaceWeightU(SolidSDF, Pos, CellSize);
		W = TMath::Clamp(W, 0.f, 1.f);
		WeightGrid[0].Cell(Index) = W;
	}
	GetDebugInfo(WeightGrid[0]);
	for (int32 Index = 0; Index < VelField[1].GetTotalCells(); Index++)
	{
		vector3di G = VelField[1].ArrayIndexToGridIndex(Index);
		vector3df Pos = vector3df(float(G.X), float(G.Y), float(G.Z)) * CellSize + VelocityStart[1];
		float W = 1.f - GetFaceWeightV(SolidSDF, Pos, CellSize);
		W = TMath::Clamp(W, 0.f, 1.f);
		WeightGrid[1].Cell(Index) = W;
	}
	GetDebugInfo(WeightGrid[1]);
	for (int32 Index = 0; Index < VelField[2].GetTotalCells(); Index++)
	{
		vector3di G = VelField[2].ArrayIndexToGridIndex(Index);
		vector3df Pos = vector3df(float(G.X), float(G.Y), float(G.Z)) * CellSize + VelocityStart[2];
		float W = 1.f - GetFaceWeightW(SolidSDF, Pos, CellSize);
		W = TMath::Clamp(W, 0.f, 1.f);
		WeightGrid[2].Cell(Index) = W;
	}
	GetDebugInfo(WeightGrid[2]);
}

void FFluidSolverFlipCPU::SolvePressure(float Dt)
{
	Pressure.Clear();

	PCGSolverParameters Parameter;
	Parameter.CellSize = CellSize;
	Parameter.DeltaTime = Dt;
	Parameter.U = &VelField[0];
	Parameter.V = &VelField[1];
	Parameter.W = &VelField[2];
	Parameter.UW = &WeightGrid[0];
	Parameter.VW = &WeightGrid[1];
	Parameter.WW = &WeightGrid[2];
	//Parameter.Marker = &Marker;
	Parameter.LiquidSDF = &LiquidSDF;
	Parameter.Pressure = &Pressure;

	PCGSolver.Solve(Parameter);
}

void FFluidSolverFlipCPU::ApplyPressure(float Dt)
{
	// Mark fluid cells
	FFluidGrid3<int8> FluidCells;
	FluidCells.Create(Dimension);
	for (int32 Index = 0; Index < FluidCells.GetTotalCells(); Index++)
	{
		if (LiquidSDF.Cell(Index) < 0.f)
		{
			// Inside liquid
			FluidCells.Cell(Index) = 1;
		}
	}

	IsValidVelocity[0].Clear();
	IsValidVelocity[1].Clear();
	IsValidVelocity[2].Clear();

	const float MinFrac = 0.01f;
	for (int32 Index = 0; Index < VelField[0].GetTotalCells(); Index++)
	{
		vector3di G = VelField[0].ArrayIndexToGridIndex(Index);
		if (WeightGrid[0].Cell(G) > 0 && IsFaceBorderValueOfGridU(G.X, G.Y, G.Z, FluidCells, int8(1)))
		{
			float P0 = Pressure.Cell(G.X - 1, G.Y, G.Z);
			float P1 = Pressure.Cell(Index);
			float Theta = TMath::Max(GetFaceWeightU(LiquidSDF, G.X, G.Y, G.Z), MinFrac);
			VelField[0].Cell(Index) += -Dt * (P1 - P0) / (CellSize.X * Theta);
			IsValidVelocity[0].Cell(Index) = 1;
		}
	}
	for (int32 Index = 0; Index < VelField[1].GetTotalCells(); Index++)
	{
		vector3di G = VelField[1].ArrayIndexToGridIndex(Index);
		if (WeightGrid[1].Cell(G) > 0 && IsFaceBorderValueOfGridV(G.X, G.Y, G.Z, FluidCells, int8(1)))
		{
			float P0 = Pressure.Cell(G.X, G.Y - 1, G.Z);
			float P1 = Pressure.Cell(Index);
			float Theta = TMath::Max(GetFaceWeightV(LiquidSDF, G.X, G.Y, G.Z), MinFrac);
			VelField[1].Cell(Index) += -Dt * (P1 - P0) / (CellSize.Y * Theta);
			IsValidVelocity[1].Cell(Index) = 1;
		}
	}
	for (int32 Index = 0; Index < VelField[2].GetTotalCells(); Index++)
	{
		vector3di G = VelField[2].ArrayIndexToGridIndex(Index);
		if (WeightGrid[2].Cell(G) > 0 && IsFaceBorderValueOfGridW(G.X, G.Y, G.Z, FluidCells, int8(1)))
		{
			float P0 = Pressure.Cell(G.X, G.Y, G.Z - 1);
			float P1 = Pressure.Cell(Index);
			float Theta = TMath::Max(GetFaceWeightW(LiquidSDF, G.X, G.Y, G.Z), MinFrac);
			VelField[2].Cell(Index) += -Dt * (P1 - P0) / (CellSize.Z * Theta);
			IsValidVelocity[2].Cell(Index) = 1;
		}
	}

	// Constrain velocity
	for (int32 Index = 0; Index < VelField[0].GetTotalCells(); Index++)
	{
		if (IsValidVelocity[0].Cell(Index) == 0)
		{
			VelField[0].Cell(Index) = 0.f;
		}
	}
	for (int32 Index = 0; Index < VelField[1].GetTotalCells(); Index++)
	{
		if (IsValidVelocity[1].Cell(Index) == 0)
		{
			VelField[1].Cell(Index) = 0.f;
		}
	}
	for (int32 Index = 0; Index < VelField[2].GetTotalCells(); Index++)
	{
		if (IsValidVelocity[2].Cell(Index) == 0)
		{
			VelField[2].Cell(Index) = 0.f;
		}
	}

}

void FFluidSolverFlipCPU::ConstrainVelocityField()
{
	for (int32 Index = 0; Index < VelField[0].GetTotalCells(); Index++)
	{
		if (WeightGrid[0].Cell(Index) == 0.f)
		{
			VelField[0].Cell(Index) = 0.f;
			VelFieldDelta[0].Cell(Index) = 0.f;
		}
}
	for (int32 Index = 0; Index < VelField[1].GetTotalCells(); Index++)
	{
		if (WeightGrid[1].Cell(Index) == 0)
		{
			VelField[1].Cell(Index) = 0.f;
			VelFieldDelta[1].Cell(Index) = 0.f;
		}
	}
	for (int32 Index = 0; Index < VelField[2].GetTotalCells(); Index++)
	{
		if (WeightGrid[2].Cell(Index) == 0)
		{
			VelField[2].Cell(Index) = 0.f;
			VelFieldDelta[2].Cell(Index) = 0.f;
		}
	}
}

void FFluidSolverFlipCPU::AdvectParticles(float Dt)
{
	vector3df CellSizeH = CellSize * 0.5f;
	vector3df VelocityOffset[3] =
	{
		vector3df(0.f, CellSizeH.Y, CellSizeH.Z),
		vector3df(CellSizeH.X, 0.f, CellSizeH.Z),
		vector3df(CellSizeH.X, CellSizeH.Y, 0.f)
	};
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < Particles.GetTotalParticles(); Index++)
	{
		// Update Particle Velocity
		const vector3df& P = Particles.GetParticlePosition(Index);
		vector3df RelativePU = MaxVector3d((P - VelocityOffset[0]) * InvCellSize, vector3df());
		vector3df RelativePV = MaxVector3d((P - VelocityOffset[1]) * InvCellSize, vector3df());
		vector3df RelativePW = MaxVector3d((P - VelocityOffset[2]) * InvCellSize, vector3df());

		float U0 = VelField[0].SampleByRelativePositionLinear(RelativePU);
		float V0 = VelField[1].SampleByRelativePositionLinear(RelativePV);
		float W0 = VelField[2].SampleByRelativePositionLinear(RelativePW);

		vector3df NewV = vector3df(U0, V0, W0);
		Particles.SetParticleVelocity(Index, NewV);

		// Advect particles with RK2
		vector3df SamplePosition = P + NewV * Dt * 0.5f;
		vector3df SamplePU = MaxVector3d((SamplePosition - VelocityOffset[0]) * InvCellSize, vector3df());
		vector3df SamplePV = MaxVector3d((SamplePosition - VelocityOffset[1]) * InvCellSize, vector3df());
		vector3df SamplePW = MaxVector3d((SamplePosition - VelocityOffset[2]) * InvCellSize, vector3df());

		float U1 = VelField[0].SampleByRelativePositionLinear(SamplePU);
		float V1 = VelField[1].SampleByRelativePositionLinear(SamplePV);
		float W1 = VelField[2].SampleByRelativePositionLinear(SamplePW);
		vector3df SampledVel = vector3df(U1, V1, W1);
		vector3df NewP = P + SampledVel * Dt;
		Particles.SetParticlePosition(Index, NewP);
	}
}