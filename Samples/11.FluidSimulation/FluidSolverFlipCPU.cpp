/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSolverFlipCPU.h"
#include "FluidSimRenderer.h"


#define DO_PARALLEL (0)
const float eps = 1e-9f;


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
	: PressureIteration(40)
{
	SubStep = 1;
}

FFluidSolverFlipCPU::~FFluidSolverFlipCPU()
{
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

	InvCellSize = vector3df(1.f) / CellSize;
	
	Dimension = Dim;
	VelField[0].Create(vector3di(Dim.X + 1, Dim.Y, Dim.Z));
	VelField[1].Create(vector3di(Dim.X, Dim.Y + 1, Dim.Z));
	VelField[2].Create(vector3di(Dim.X, Dim.Y, Dim.Z + 1));
	Weights[0].Create(VelField[0].GetDimension());
	Weights[1].Create(VelField[1].GetDimension());
	Weights[2].Create(VelField[2].GetDimension());
	Divergence.Create(Dim);
	Pressure.Create(Dim);
	VelFieldDelta[0].Create(VelField[0].GetDimension());
	VelFieldDelta[1].Create(VelField[1].GetDimension());
	VelFieldDelta[2].Create(VelField[2].GetDimension());
}

int32 Counter = 0;
void FFluidSolverFlipCPU::Sim(FRHI * RHI, float Dt)
{
	TIMER_RECORDER("FlipSim");
	ParticleToGrids();
	BackupVelocity();
	CalcExternalForces(Dt);
	BoundaryCheck();
	CalcVisicosity(Dt);
	CalcDivergence();
	CalcPressure(Dt);
	GradientSubstract(Dt);
	BoundaryCheck();
	GridsToParticleFLIP();
	MoveParticles(Dt);
}

void FFluidSolverFlipCPU::GetSampleCellAndWeightsByPosition(const vector3df& Position, TVector<vector3di>& Cells, TVector<float>& Weights)
{
	Cells.resize(8);
	Weights.resize(8);

	vector3df CellPos = (Position - Origin) * InvCellSize;
	vector3df CellPosMin = CellPos - vector3df(0.5f, 0.5f, 0.5f);
	vector3di CellIndexMin = Floor(CellPosMin);
	vector3df CellPosMinFrac = CellPosMin - vector3df((float)CellIndexMin.X, (float)CellIndexMin.Y, (float)CellIndexMin.Z);

	Cells[0] = CellIndexMin + vector3di(0, 0, 0);
	Cells[1] = CellIndexMin + vector3di(1, 0, 0);
	Cells[2] = CellIndexMin + vector3di(0, 1, 0);
	Cells[3] = CellIndexMin + vector3di(1, 1, 0);
	Cells[4] = CellIndexMin + vector3di(0, 0, 1);
	Cells[5] = CellIndexMin + vector3di(1, 0, 1);
	Cells[6] = CellIndexMin + vector3di(0, 1, 1);
	Cells[7] = CellIndexMin + vector3di(1, 1, 1);

	const vector3di CellMin = vector3di();
	const vector3di CellMax = vector3di(Dimension.X - 1, Dimension.Y - 1, Dimension.Z - 1);
	Cells[0] = ClampVector3d(Cells[0], CellMin, CellMax);
	Cells[1] = ClampVector3d(Cells[1], CellMin, CellMax);
	Cells[2] = ClampVector3d(Cells[2], CellMin, CellMax);
	Cells[3] = ClampVector3d(Cells[3], CellMin, CellMax);
	Cells[4] = ClampVector3d(Cells[4], CellMin, CellMax);
	Cells[5] = ClampVector3d(Cells[5], CellMin, CellMax);
	Cells[6] = ClampVector3d(Cells[6], CellMin, CellMax);
	Cells[7] = ClampVector3d(Cells[7], CellMin, CellMax);

	Weights[0] = (1.f - CellPosMinFrac.X) * (1.f - CellPosMinFrac.Y) * (1.f - CellPosMinFrac.Z);
	Weights[1] = CellPosMinFrac.X * (1.f - CellPosMinFrac.Y) * (1.f - CellPosMinFrac.Z);
	Weights[2] = (1.f - CellPosMinFrac.X) * CellPosMinFrac.Y * (1.f - CellPosMinFrac.Z);
	Weights[3] = CellPosMinFrac.X * CellPosMinFrac.Y * (1.f - CellPosMinFrac.Z);
	Weights[4] = (1.f - CellPosMinFrac.X) * (1.f - CellPosMinFrac.Y) * CellPosMinFrac.Z;
	Weights[5] = CellPosMinFrac.X * (1.f - CellPosMinFrac.Y) * CellPosMinFrac.Z;
	Weights[6] = (1.f - CellPosMinFrac.X) * CellPosMinFrac.Y * CellPosMinFrac.Z;
	Weights[7] = CellPosMinFrac.X * CellPosMinFrac.Y * CellPosMinFrac.Z;
}

float FFluidSolverFlipCPU::InterporlateVelocity(int32 Component, const FFluidGrid3<float>& VelGrid, const vector3df& Position)
{
	TVector<vector3di> Cells;
	TVector<float> Wt;
	Cells.resize(8);
	Wt.resize(8);

	vector3df Offset;
	if (Component == 0)
	{
		Offset = vector3df(0.f, 0.5f, 0.5f);
	}
	else if (Component == 1)
	{
		Offset = vector3df(0.5f, 0.f, 0.5f);
	}
	else if (Component == 2)
	{
		Offset = vector3df(0.5f, 0.5f, 0.f);
	}
	else
	{
		TI_ASSERT(0);
	}


	vector3df CellPos = (Position - Origin) * InvCellSize;
	vector3df CellPosMin = CellPos - Offset;
	vector3di CellIndexMin = Floor(CellPosMin);
	vector3df CellPosMinFrac = CellPosMin - vector3df((float)CellIndexMin.X, (float)CellIndexMin.Y, (float)CellIndexMin.Z);

	Cells[0] = CellIndexMin + vector3di(0, 0, 0);
	Cells[1] = CellIndexMin + vector3di(1, 0, 0);
	Cells[2] = CellIndexMin + vector3di(0, 1, 0);
	Cells[3] = CellIndexMin + vector3di(1, 1, 0);
	Cells[4] = CellIndexMin + vector3di(0, 0, 1);
	Cells[5] = CellIndexMin + vector3di(1, 0, 1);
	Cells[6] = CellIndexMin + vector3di(0, 1, 1);
	Cells[7] = CellIndexMin + vector3di(1, 1, 1);

	const vector3di CellMin = vector3di();
	const vector3di CellMax = vector3di(Dimension.X - 1, Dimension.Y - 1, Dimension.Z - 1);
	Cells[0] = ClampVector3d(Cells[0], CellMin, CellMax);
	Cells[1] = ClampVector3d(Cells[1], CellMin, CellMax);
	Cells[2] = ClampVector3d(Cells[2], CellMin, CellMax);
	Cells[3] = ClampVector3d(Cells[3], CellMin, CellMax);
	Cells[4] = ClampVector3d(Cells[4], CellMin, CellMax);
	Cells[5] = ClampVector3d(Cells[5], CellMin, CellMax);
	Cells[6] = ClampVector3d(Cells[6], CellMin, CellMax);
	Cells[7] = ClampVector3d(Cells[7], CellMin, CellMax);

	Wt[0] = (1.f - CellPosMinFrac.X) * (1.f - CellPosMinFrac.Y) * (1.f - CellPosMinFrac.Z);
	Wt[1] = CellPosMinFrac.X * (1.f - CellPosMinFrac.Y) * (1.f - CellPosMinFrac.Z);
	Wt[2] = (1.f - CellPosMinFrac.X) * CellPosMinFrac.Y * (1.f - CellPosMinFrac.Z);
	Wt[3] = CellPosMinFrac.X * CellPosMinFrac.Y * (1.f - CellPosMinFrac.Z);
	Wt[4] = (1.f - CellPosMinFrac.X) * (1.f - CellPosMinFrac.Y) * CellPosMinFrac.Z;
	Wt[5] = CellPosMinFrac.X * (1.f - CellPosMinFrac.Y) * CellPosMinFrac.Z;
	Wt[6] = (1.f - CellPosMinFrac.X) * CellPosMinFrac.Y * CellPosMinFrac.Z;
	Wt[7] = CellPosMinFrac.X * CellPosMinFrac.Y * CellPosMinFrac.Z;

	// For debug, all weights' sum should be 1
	float W = Wt[0] + Wt[1] + Wt[2] + Wt[3] + Wt[4] + Wt[5] + Wt[6] + Wt[7];
	
	float VelComponent = 0.f;
	for (int32 i = 0; i < 8; i++)
	{
		VelComponent += VelGrid.Cell(Cells[i])* Wt[i];
	}
	return VelComponent;
}

void FFluidSolverFlipCPU::ParticleToGrids()
{
	// Clear vel and weight to 0
	VelField[0].Clear();
	VelField[1].Clear();
	VelField[2].Clear();

	const vector3df HalfCellSize = CellSize * 0.5f;

	vector3df VelStartPos[3];
	VelStartPos[0] = vector3df(0.f, HalfCellSize.Y, HalfCellSize.Z);
	VelStartPos[1] = vector3df(HalfCellSize.X, 0.f, HalfCellSize.Z);
	VelStartPos[2] = vector3df(HalfCellSize.X, HalfCellSize.Y, 0.f);

	Weights[0].Clear();
	Weights[1].Clear();
	Weights[2].Clear();

	float r[3], rsq[3], coef1[3], coef2[3], coef3[3];
	for (int32 i = 0; i < 3; i++)
	{
		r[i] = CellSize[i];
		rsq[i] = r[i] * r[i];
		coef1[i] = (4.f / 9.f) * (1.f / (rsq[i] * rsq[i] * rsq[i]));
		coef2[i] = (17.f / 9.f) * (1.f / (rsq[i] * rsq[i]));
		coef3[i] = (22.f / 9.f) * (1.f / rsq[i]);
	}


#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0 ; Index < Particles.GetTotalParticles(); Index ++)
	{
		for (int32 VelIndex = 0; VelIndex < 3; VelIndex++)
		{
			const vector3df& V = Particles.GetParticleVelocity(Index);
			vector3df PosInGrid = Particles.GetParticlePosition(Index) - VelStartPos[VelIndex];
			vector3di IndexInGrid = Floor(PosInGrid * InvCellSize);
			vector3di GridMin = vector3di(IndexInGrid.X - 1, IndexInGrid.Y - 1, IndexInGrid.Z - 1);
			vector3di GridMax = vector3di(IndexInGrid.X + 1, IndexInGrid.Y + 1, IndexInGrid.Z + 1);
			const vector3di& GridDim = VelField[VelIndex].GetDimension();
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
						if (DisSQ < rsq[i])
						{
							float Weight =
								1.f - coef1[VelIndex] * DisSQ * DisSQ * DisSQ +
								coef2[VelIndex] * DisSQ * DisSQ -
								coef3[VelIndex] * DisSQ;
#if DO_PARALLEL
#pragma omp critical
#endif
							{
								VelField[VelIndex].Cell(i, j, k) += V[VelIndex] * Weight;
								Weights[VelIndex].Cell(i, j, k) += Weight;
							}
						}
					}
				}
			}
		}
	}

#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[0].GetTotalCells(); Index++)
	{
		if (Weights[0].Cell(Index) > eps)
			VelField[0].Cell(Index) = VelField[0].Cell(Index) / Weights[0].Cell(Index);
	}

#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[1].GetTotalCells(); Index++)
	{
		if (Weights[1].Cell(Index) > eps)
			VelField[1].Cell(Index) = VelField[1].Cell(Index) / Weights[1].Cell(Index);
	}

#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[2].GetTotalCells(); Index++)
	{
		if (Weights[2].Cell(Index) > eps)
			VelField[2].Cell(Index) = VelField[2].Cell(Index) / Weights[2].Cell(Index);
	}
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
	const vector3df Gravity = vector3df(0.f, 0.f, -9.8f);
	const vector3df DV = Gravity * Dt;
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[2].GetTotalCells(); Index++)
	{
		if (Weights[2].Cell(Index) > eps)
		{
			VelField[2].Cell(Index) += DV.Z;
		}
	}
}

void FFluidSolverFlipCPU::CalcVisicosity(float Dt)
{

}

void FFluidSolverFlipCPU::CalcDivergence()
{
	TI_ASSERT(Divergence.GetDimension() == Pressure.GetDimension());
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < Divergence.GetTotalCells(); Index++)
	{
		vector3di GridIndex = Divergence.ArrayIndexToGridIndex(Index);

		float ULeft = VelField[0].SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z);
		float URight = VelField[0].SafeCell(GridIndex.X + 1, GridIndex.Y, GridIndex.Z);
		float VFront = VelField[1].SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z);
		float VBack = VelField[1].SafeCell(GridIndex.X, GridIndex.Y + 1, GridIndex.Z);
		float WUp = VelField[2].SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z);
		float WDown = VelField[2].SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z + 1);

		//float L = (GridIndex.X == 0) ? -V.X : VLeft.X;
		//float R = (GridIndex.X == (Dimension.X - 1)) ? -V.X : VRight.X;
		//float F = (GridIndex.Y == 0) ? -V.Y : VFront.Y;
		//float B = (GridIndex.Y == (Dimension.Y - 1)) ? -V.Y : VBack.Y;
		//float U = (GridIndex.Z == 0) ? -V.Z : VUp.Z;
		//float D = (GridIndex.Z == (Dimension.Z - 1)) ? -V.Z : VDown.Z;
		float L = ULeft;
		float R = URight;
		float F = VFront;
		float B = VBack;
		float U = WUp;
		float D = WDown;

		float Div = (R - L) * InvCellSize.X + (B - F) * InvCellSize.Y + (D - U) * InvCellSize.Z;
		Divergence.Cell(Index) = Div;
	}
}

void FFluidSolverFlipCPU::CalcPressure(float Dt)
{
	Pressure.Clear();
	vector3df InvCellSizeSQ = InvCellSize * InvCellSize;
	for (int32 i = 0; i < PressureIteration; i++)
	{
#if DO_PARALLEL
#pragma omp parallel for
#endif
		for (int32 Index = 0; Index < Pressure.GetTotalCells(); Index++)
		{
			vector3di GridIndex = Pressure.ArrayIndexToGridIndex(Index);

			float PL = Pressure.SafeCell(GridIndex.X - 1, GridIndex.Y, GridIndex.Z);
			float PR = Pressure.SafeCell(GridIndex.X + 1, GridIndex.Y, GridIndex.Z);
			float PF = Pressure.SafeCell(GridIndex.X, GridIndex.Y - 1, GridIndex.Z);
			float PB = Pressure.SafeCell(GridIndex.X, GridIndex.Y + 1, GridIndex.Z);
			float PU = Pressure.SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z - 1);
			float PD = Pressure.SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z + 1);

			float Div = Divergence.Cell(Index);

			float P = 
				(
					(PL + PR) * InvCellSizeSQ.X + 
					(PF + PB) * InvCellSizeSQ.Y + 
					(PU + PD) * InvCellSizeSQ.Z - Div
				) / (2.f * (InvCellSizeSQ.X + InvCellSizeSQ.Y + InvCellSizeSQ.Z) );

			Pressure.Cell(Index) = P;
		}
	}
}

void FFluidSolverFlipCPU::GradientSubstract(float Dt)
{
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < Pressure.GetTotalCells(); Index++)
	{
		vector3di GridIndex = Pressure.ArrayIndexToGridIndex(Index);

		float PL = Pressure.SafeCell(GridIndex.X - 1, GridIndex.Y, GridIndex.Z);
		float PR = Pressure.SafeCell(GridIndex.X + 1, GridIndex.Y, GridIndex.Z);
		float PF = Pressure.SafeCell(GridIndex.X, GridIndex.Y - 1, GridIndex.Z);
		float PB = Pressure.SafeCell(GridIndex.X, GridIndex.Y + 1, GridIndex.Z);
		float PU = Pressure.SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z - 1);
		float PD = Pressure.SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z + 1);

		float& U = VelField[0].Cell(Index);
		float& V = VelField[1].Cell(Index);
		float& W = VelField[2].Cell(Index);

		vector3df Gradient;
		Gradient.X = (PR - PL) * InvCellSize.X * 0.5f;
		Gradient.Y = (PB - PF) * InvCellSize.Y * 0.5f;
		Gradient.Z = (PD - PU) * InvCellSize.Z * 0.5f;

		U -= Gradient.X * Dt;
		V -= Gradient.Y * Dt;
		W -= Gradient.Z * Dt;
	}
}

void FFluidSolverFlipCPU::GridsToParticlePIC()
{
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < Particles.GetTotalParticles(); Index++)
	{
		const vector3df& P = Particles.GetParticlePosition(Index);

		float U = InterporlateVelocity(0, VelField[0], P);
		float V = InterporlateVelocity(1, VelField[1], P);
		float W = InterporlateVelocity(2, VelField[2], P);

		Particles.SetParticleVelocity(Index, vector3df(U, V, W));
	}
}

void FFluidSolverFlipCPU::GridsToParticleFLIP()
{
	// Calc the difference with the backup result
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[0].GetTotalCells(); Index++)
	{
		VelFieldDelta[0].Cell(Index) = VelField[0].Cell(Index) - VelFieldDelta[0].Cell(Index);
		VelFieldDelta[1].Cell(Index) = VelField[1].Cell(Index) - VelFieldDelta[1].Cell(Index);
		VelFieldDelta[2].Cell(Index) = VelField[2].Cell(Index) - VelFieldDelta[2].Cell(Index);
	}

	// Interpolate difference and add to particle velocity
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < Particles.GetTotalParticles(); Index++)
	{
		const vector3df& P = Particles.GetParticlePosition(Index);
		const vector3df& V = Particles.GetParticleVelocity(Index);

		float UDelta = InterporlateVelocity(0, VelFieldDelta[0], P);
		float VDelta = InterporlateVelocity(1, VelFieldDelta[1], P);
		float WDelta = InterporlateVelocity(2, VelFieldDelta[2], P);

		Particles.SetParticleVelocity(Index, V + vector3df(UDelta, VDelta, WDelta));
	}
}

void FFluidSolverFlipCPU::MoveParticles(float Dt)
{
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < Particles.GetTotalParticles(); Index++)
	{
		const vector3df& P = Particles.GetParticlePosition(Index);
		const vector3df& V = Particles.GetParticleVelocity(Index);

		vector3df NewP = P + V * Dt;
		Particles.SetParticlePosition(Index, NewP);
	}
}

void FFluidSolverFlipCPU::BoundaryCheck()
{
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[0].GetTotalCells(); Index++)
	{
		uint32 BoundaryInfoU = VelField[0].GetBoudaryTypeInfo(Index);
		float& U = VelField[0].Cell(Index);
		if ((BoundaryInfoU & Boundary_Left) != 0)
			U = TMath::Max(0.f, U);
		else if ((BoundaryInfoU & Boundary_Right) != 0)
			U = TMath::Min(0.f, U);

		uint32 BoundaryInfoV = VelField[1].GetBoudaryTypeInfo(Index);
		float& V = VelField[1].Cell(Index);
		if ((BoundaryInfoV & Boundary_Front) != 0)
			V = TMath::Max(0.f, V);
		else if ((BoundaryInfoV & Boundary_Back) != 0)
			V = TMath::Min(0.f, V);

		uint32 BoundaryInfoW = VelField[2].GetBoudaryTypeInfo(Index);
		float& W = VelField[2].Cell(Index);
		if ((BoundaryInfoW & Boundary_Up) != 0)
			W = TMath::Max(0.f, W);
		else if ((BoundaryInfoW & Boundary_Bottom) != 0)
			W = TMath::Min(0.f, W);
	}
}