/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSolverPbf.h"
#include "FluidSimRenderer.h"

FFluidSolverPbf::FFluidSolverPbf()
	: RestDenstiy(1000.f)
	, Viscosity(0.01f)
	, Epsilon(600.f)
	, TotalCells(0)
	, CellSize(1.f)
{
}

FFluidSolverPbf::~FFluidSolverPbf()
{
}

void FFluidSolverPbf::CreateNeighborSearchGrid(const vector3df& BoundSize, float ParticleSeperation)
{
	CellSize = ParticleSeperation;
	const float CellSizeInv = 1.f / CellSize;

	Dim.X = (int32)(BoundSize.X * CellSizeInv + 1);
	Dim.Y = (int32)(BoundSize.Y * CellSizeInv + 1);
	Dim.Z = (int32)(BoundSize.Z * CellSizeInv + 1);

	TotalCells = Dim.X * Dim.Y * Dim.Z;
}

void FFluidSolverPbf::CreateParticles(
	const aabbox3df& InParticleBox,
	float InParticleSeperation,
	float InParticleMass)
{
	Particles.CreateParticlesInBox(InParticleBox, InParticleSeperation, InParticleMass);
	Flag |= DirtyParticles;
}
