/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FluidSolver.h"
#include "FluidParticle.h"

template<class T>
class FGrid3
{
public:
	FGrid3()
	{
	}
	FGrid3(const vector3di& Dim)
	{
		Create(Dim);
	}

	void Create(const vector3di& Dim)
	{
		Dimension = Dim;

		Cells.clear();
		Cells.resize(Dim.X * Dim.Y * Dim.Z);
	}

	void Clear()
	{
		memset(Cells.data(), 0, Cells.size() * sizeof(T));
	}

	T& Cell(const vector3di& Index)
	{
		return Cell(Index.X, Index.Y, Index.Z);
	}
	const T& Cell(const vector3di& Index) const
	{
		return Cell(Index.X, Index.Y, Index.Z);
	}

	T& Cell(int32 i, int32 j, int32 k)
	{
		return Cells[GridIndexToArrayIndex(i, j, k)];
	}
	const T& Cell(int32 i, int32 j, int32 k) const
	{
		return Cells[GridIndexToArrayIndex(i, j, k)];
	}
private:
	void ValidateGridIndex(int32 i, int32 j, int32 k)
	{
		TI_ASSERT(i >= 0 && i < Dimension.X);
		TI_ASSERT(j >= 0 && j < Dimension.Y);
		TI_ASSERT(k >= 0 && k < Dimension.Z);
	}
	int32 GridIndexToArrayIndex(int32 i, int32 j, int32 k)
	{
		ValidateGridIndex(i, j, k);
		int32 Index = k * Dimension.X * Dimension.Y + j * Dimension.X + i;
		return Index;
	}
private:
	vector3di Dimension;
	TVector<T> Cells;
};

class FFluidSolverFlipCPU : public FFluidSolver
{
public:

	FFluidSolverFlipCPU();
	virtual ~FFluidSolverFlipCPU();

	void CreateGrid(const vector3di& Dim);
	virtual void Sim(FRHI* RHI, float Dt) override;

private:
	void ParticleToGrids();
	void CalcExternalForces(float Dt);
	void CalcVisicosity(float Dt);
	void CalcPressure(float Dt);
	void GridsToParticle();
	void MoveParticles();

private:
	FFluidParticle Particles;

	vector3df Origin;
	vector3df CellSize;
	vector3df InvCellSize;
	vector3di Dimension;
	FGrid3<vector3df> Vel;
	FGrid3<float> Weight;
	FGrid3<float> Pressure;
};
