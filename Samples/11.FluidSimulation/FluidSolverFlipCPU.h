/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FluidSolver.h"
#include "FluidParticle.h"
#include "FluidGrid.h"

class FFluidSolverFlipCPU : public FFluidSolver
{
public:

	FFluidSolverFlipCPU();
	virtual ~FFluidSolverFlipCPU();

	void CreateParticles(
		const aabbox3df& InParticleBox,
		float InParticleSeperation,
		float InParticleMass);
	void CreateGrid(const vector3di& Dim);
	virtual void Sim(FRHI* RHI, float Dt) override;

	virtual int32 GetNumParticles() override
	{
		return Particles.GetTotalParticles();
	}
	const TVector<vector3df>& GetSimulatedPositions()
	{
		return Particles.GetParticlePositions();
	}
private:
	void ParticleToGrids();
	void CalcExternalForces(float Dt);
	void CalcVisicosity(float Dt);
	void CalcDivergence();
	void CalcPressure(float Dt);
	void GradientSubstract();
	void GridsToParticle();
	void MoveParticles();
	void BoundaryCheck();

private:
	FFluidParticle Particles;

	vector3df Origin;
	vector3df CellSize;
	vector3df InvCellSize;
	vector3di Dimension;
	FFluidGrid3<vector3df> Vel;
	FFluidGrid3<float> Weight;
	FFluidGrid3<float> Divergence;
	FFluidGrid3<float> Pressure;

	int32 PressureIteration;
};
