/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSolverFlipCPU.h"
#include "FluidSimRenderer.h"


#define DO_PARALLEL (0)


template < class T>
inline vector3d<T> ClampVector3d(const vector3d<T>& V, const vector3d<T>& Min, const vector3d<T>& Max)
{
	vector3d<T> Result;
	Result.X = TMath::Clamp(V.X, Min.X, Max.X);
	Result.Y = TMath::Clamp(V.Y, Min.Y, Max.Y);
	Result.Z = TMath::Clamp(V.Z, Min.Z, Max.Z);
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
	Vel.Create(Dim);
	Weight.Create(Dim);
	Divergence.Create(Dim);
	Pressure.Create(Dim);
}

int32 Counter = 0;
void FFluidSolverFlipCPU::Sim(FRHI * RHI, float Dt)
{
	TIMER_RECORDER("FlipSim");
	ParticleToGrids();
	CalcExternalForces(Dt);
	BoundaryCheck();
	CalcVisicosity(Dt);
	CalcDivergence();
	CalcPressure(Dt);
	GradientSubstract();
	BoundaryCheck();
	GridsToParticle();
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

void FFluidSolverFlipCPU::ParticleToGrids()
{
	// Clear vel and weight to 0
	Vel.Clear();
	Weight.Clear();

#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0 ; Index < Particles.GetTotalParticles(); Index ++)
	{
		const vector3df& P = Particles.GetParticlePosition(Index);
		const vector3df& V = Particles.GetParticleVelocity(Index);

		TVector<vector3di> Cells;
		TVector<float> Weights;
		GetSampleCellAndWeightsByPosition(P, Cells, Weights);

		// For debug, all weights' sum should be 1
		float W = Weights[0] + Weights[1] + Weights[2] + Weights[3] + Weights[4] + Weights[5] + Weights[6] + Weights[7];

#if DO_PARALLEL
#pragma omp critical
#endif
		{
			for (int32 i = 0; i < 8; i++)
			{
				Vel.Cell(Cells[i]) += V * Weights[i];
				Weight.Cell(Cells[i]) += Weights[i];
			}
		}
	}

#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < Vel.GetTotalCells(); Index++)
	{
		if (Weight.Cell(Index) > 0.f)
			Vel.Cell(Index) = Vel.Cell(Index) / Weight.Cell(Index);
	}
}

void FFluidSolverFlipCPU::CalcExternalForces(float Dt)
{
	const vector3df Gravity = vector3df(0.f, 0.f, -9.8f);
	const vector3df DV = Gravity * Dt;
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < Vel.GetTotalCells(); Index++)
	{
		if (Weight.Cell(Index) > 0.f)
		{
			Vel.Cell(Index) = Vel.Cell(Index) + DV;
		}
	}
}

void FFluidSolverFlipCPU::CalcVisicosity(float Dt)
{

}

void FFluidSolverFlipCPU::CalcDivergence()
{
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < Vel.GetTotalCells(); Index++)
	{
		vector3di GridIndex = Vel.ArrayIndexToGridIndex(Index);

		const vector3df& V = Vel.Cell(Index);
		const vector3df& VLeft = Vel.SafeCell(GridIndex.X - 1, GridIndex.Y, GridIndex.Z);
		const vector3df& VRight = Vel.SafeCell(GridIndex.X + 1, GridIndex.Y, GridIndex.Z);
		const vector3df& VFront = Vel.SafeCell(GridIndex.X, GridIndex.Y - 1, GridIndex.Z);
		const vector3df& VBack = Vel.SafeCell(GridIndex.X, GridIndex.Y + 1, GridIndex.Z);
		const vector3df& VUp = Vel.SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z - 1);
		const vector3df& VDown = Vel.SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z + 1);

		float L = (GridIndex.X == 0) ? -V.X : VLeft.X;
		float R = (GridIndex.X == (Dimension.X - 1)) ? -V.X : VRight.X;
		float F = (GridIndex.Y == 0) ? -V.Y : VFront.Y;
		float B = (GridIndex.Y == (Dimension.Y - 1)) ? -V.Y : VBack.Y;
		float U = (GridIndex.Z == 0) ? -V.Z : VUp.Z;
		float D = (GridIndex.Z == (Dimension.Z - 1)) ? -V.Z : VDown.Z;

		float Div = ((R - L) * InvCellSize.X + (B - F) * InvCellSize.Y + (D - U) * InvCellSize.Z) * 0.5f;
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
		for (int32 Index = 0; Index < Vel.GetTotalCells(); Index++)
		{
			vector3di GridIndex = Vel.ArrayIndexToGridIndex(Index);

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

void FFluidSolverFlipCPU::GradientSubstract()
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

		const vector3df& V = Vel.Cell(Index);

		vector3df Gradient;
		Gradient.X = (PR - PL) * InvCellSize.X * 0.5f;
		Gradient.Y = (PB - PF) * InvCellSize.Y * 0.5f;
		Gradient.Z = (PD - PU) * InvCellSize.Z * 0.5f;
		
		vector3df NewV = V - Gradient;

		Vel.Cell(Index) = NewV;
	}
}

void FFluidSolverFlipCPU::GridsToParticle()
{
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < Particles.GetTotalParticles(); Index++)
	{
		const vector3df& P = Particles.GetParticlePosition(Index);

		TVector<vector3di> Cells;
		TVector<float> Weights;
		GetSampleCellAndWeightsByPosition(P, Cells, Weights);

		// For debug, all weights' sum should be 1
		float W = Weights[0] + Weights[1] + Weights[2] + Weights[3] + Weights[4] + Weights[5] + Weights[6] + Weights[7];

		vector3df NewV;
		for (int32 i = 0; i < 8; i++)
		{
			NewV += Vel.Cell(Cells[i]) * Weights[i];
		}
		Particles.SetParticleVelocity(Index, NewV);
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
	for (int32 Index = 0; Index < Vel.GetTotalCells(); Index++)
	{
		if (Weight.Cell(Index) > 0.f)
		{
			uint32 BoundaryInfo = Vel.GetBoudaryTypeInfo(Index);
			if (BoundaryInfo != Boundary_None)
			{
				vector3df& V = Vel.Cell(Index);
				if ((BoundaryInfo & Boundary_Left) != 0 && V.X < 0.f)
					V.X = 0.f;
				else if ((BoundaryInfo & Boundary_Right) != 0 && V.X > 0.f)
					V.X = 0.f;

				if ((BoundaryInfo & Boundary_Front) != 0 && V.Y < 0.f)
					V.Y = 0.f;
				else if ((BoundaryInfo & Boundary_Back) != 0 && V.Y > 0.f)
					V.Y = 0.f;
					
				if ((BoundaryInfo & Boundary_Up) != 0 && V.Z < 0.f)
					V.Z = 0.f;
				else if ((BoundaryInfo & Boundary_Bottom) != 0 && V.Z > 0.f)
					V.Z = 0.f;
			}
		}
	}
}