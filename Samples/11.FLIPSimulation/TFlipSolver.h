/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TMACGrid;
class TFluidsParticles;
class TFlipSolver
{
public:
	TFlipSolver();
	~TFlipSolver();

	void InitGrid(const vector3di& InSize, float InSeperation);
	void CreateParticlesInSphere(const vector3df& InCenter, float InRadius, float InSeperation);

	void DoSimulation(float Dt);

protected:
	void TransferFromParticlesToGrids();
	void ComputeForces();
	void ComputeViscosity();
	void ComputePressure();
	void ComputeAdvection();
	void TransferFromGridsToParticles();
	void ExtrapolateVelocityToAir();
	void ApplyBoundaryCondition();
	void MoveParticles();

protected:
	TMACGrid* Grid;
	TFluidsParticles* Particles;
};