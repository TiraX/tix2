/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSolverFlipCPU.h"
#include "FluidSimRenderer.h"


#define DO_PARALLEL (1)


template < class T>
inline vector3d<T> ClampVector3d(const vector3d<T>& V, const vector3d<T>& Min, const vector3d<T>& Max)
{
	vector3d<T> Result;
	Result.X = Clamp(V.X, Min.X, Max.X);
	Result.Y = Clamp(V.Y, Min.Y, Max.Y);
	Result.Z = Clamp(V.Z, Min.Z, Max.Z);
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
{
	SubStep = 1;
}

FFluidSolverFlipCPU::~FFluidSolverFlipCPU()
{
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
	Pressure.Create(Dim);
}

void FFluidSolverFlipCPU::Sim(FRHI * RHI, float Dt)
{
	ParticleToGrids();
	CalcExternalForces(Dt);
	CalcVisicosity(Dt);
	CalcPressure(Dt);
	GridsToParticle();
	MoveParticles();
}

void FFluidSolverFlipCPU::ParticleToGrids()
{
	// Clear vel and weight to 0
	Vel.Clear();
	Weight.Clear();

#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (const auto& P : ParticlePositions)
	{
		vector3di Cells[8];
		float Weights[8];

		vector3df CellPos = (P - Origin) * InvCellSize;
		vector3df CellPosMin = CellPos - vector3df(0.5f, 0.5f, 0.5f);
		vector3di CellIndexMin = Floor(CellPosMin);
		vector3df CellPosMinFrac = CellPosMin - vector3df(CellIndexMin.X, CellIndexMin.Y, CellIndexMin.Z);

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

		// For debug
		float W = Weights[0] + Weights[1] + Weights[2] + Weights[3] + Weights[4] + Weights[5] + Weights[6] + Weights[7];

#if DO_PARALLEL
#pragma omp critical
#endif
		{
			for (int32 i = 0; i < 8; i++)
			{
				Vel.Cell(Cells[i]) += 0;
			}
			//we need speed here.'
			TI_ASSERT(0);
		}

	}
}

void FFluidSolverFlipCPU::CalcExternalForces(float Dt)
{

}

void FFluidSolverFlipCPU::CalcVisicosity(float Dt)
{

}

void FFluidSolverFlipCPU::CalcPressure(float Dt)
{

}

void FFluidSolverFlipCPU::GridsToParticle()
{

}

void FFluidSolverFlipCPU::MoveParticles()
{

}