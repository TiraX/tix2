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
	void BackupVelocity();
	void CalcExternalForces(float Dt);
	void CalcVisicosity(float Dt);
	void CalcDivergence();
	void CalcPressure(float Dt);
	void GradientSubstract(float Dt);
	void GridsToParticlePIC();
	void GridsToParticleFLIP();
	void MoveParticles(float Dt);
	void BoundaryCheck();
	void GetSampleCellAndWeightsByPosition(const vector3df& Position, TVector<vector3di>& Cells, TVector<float>& Weights);
	float InterporlateVelocity(int32 Component, const FFluidGrid3<float>& VelGrid, const vector3df& Position);	// Component = 0,1,2 mapto U, V, W

private:
	FFluidParticle Particles;

	vector3df Origin;
	vector3df CellSize;
	vector3df InvCellSize;
	vector3di Dimension;

	// MAC Grid 
	FFluidGrid3<float> VelField[3];
	FFluidGrid3<float> Weights[3];
	FFluidGrid3<float> Divergence;
	FFluidGrid3<float> Pressure;
	FFluidGrid3<float> VelFieldDelta[3];

	int32 PressureIteration;
};
