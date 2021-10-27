/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FluidSolver.h"
#include "FluidParticle.h"
#include "FluidGrid.h"
#include "PCGSolver.h"
#include "GeometrySDF.h"

// Make a new implementation with pcg
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
	void AddCollision(FGeometrySDF* CollisionSDF);
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
	void UpdateLiquidSDF();
	void AdvectVelocityField();
	void MarkCells();
	void ExtrapolateVelocityField();
	void BackupVelocity();
	void CalcExternalForces(float Dt);
	void CalcVisicosity(float Dt);
	void ComputeWeights();
	void SolvePressure(float Dt);
	void ApplyPressure(float Dt);
	void ConstrainVelocityField();
	void AdvectParticles(float Dt);
	//void BoundaryCheck();
	//void GetSampleCellAndWeightsByPosition(const vector3df& Position, TVector<vector3di>& Cells, TVector<float>& Weights);
	//float InterporlateVelocity(int32 Component, const FFluidGrid3<float>& VelGrid, const vector3df& Position);	// Component = 0,1,2 mapto U, V, W

private:
	FFluidParticle Particles;

	vector3df Origin;
	vector3df CellSize;
	vector3df InvCellSize;
	vector3di Dimension;
	float ParticleRadius;

	// MAC Grid 
	FFluidGrid3<float> VelField[3];
	//FFluidGrid3<int32> Marker;
	FFluidGrid3<float> Divergence;
	FFluidGrid3<float> Pressure;
	FFluidGrid3<float> VelFieldDelta[3];
	FFluidGrid3<int8> IsValidVelocity[3];
	FFluidGrid3<float> WeightGrid[3];
	FFluidGrid3<float> LiquidSDF;

	//FGeometrySDF* SolidSDF;
	FFluidGrid3<float> SolidSDF;

	FPCGSolver PCGSolver;
};
