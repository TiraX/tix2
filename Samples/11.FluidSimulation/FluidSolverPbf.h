/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FluidSolver.h"
#include "FluidParticle.h"

class FFluidSolverPbf : public FFluidSolver
{
public:
	FFluidSolverPbf();
	virtual ~FFluidSolverPbf();

	void CreateNeighborSearchGrid(const vector3df& BoundSize, float ParticleSeperation);
	void CreateParticles(
		const aabbox3df& InParticleBox,
		float InParticleSeperation,
		float InParticleMass);

	virtual int32 GetNumParticles() override
	{
		return Particles.GetTotalParticles();
	}
private:

protected:
	FFluidParticle Particles;

	// Pbf parameters
	float RestDenstiy;
	float Viscosity;
	float Epsilon;

	// Neighbor search parameters
	int32 TotalCells;
	float CellSize;
	vector3di Dim;
};
