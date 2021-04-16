/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "TFdmIccgSolver.h"

class TMACGrid;
class TFluidsParticles;
class TFlipSolver
{
public:
	TFlipSolver();
	~TFlipSolver();

	void InitSolver(const vector3di& InSize, float InSeperation);
	void CreateParticlesInSphere(const vectype& InCenter, float InRadius, float InSeperation);

	void DoSimulation(float Dt);
	void ExportParticles(int32 Frame);

protected:
	void TransferFromParticlesToGrids();
	void ComputeForces(float Dt);
	void ComputeParticlesForces(float Dt);
	void ComputeViscosity();
	void ComputePressure();
	void ComputeParticlesPressure(float Dt);
	void ComputeAdvection();
	void TransferFromGridsToParticles();
	void ExtrapolateVelocityToAir();
	void ApplyBoundaryCondition();
	void MoveParticles(float Dt);

	void BuildLinearSystem();
	void ApplyPressureGradient();


protected:
	TMACGrid* Grid;
	TFluidsParticles* Particles;

	// Matrix and vector for linear system
	TArray3<FdmMatrixRow3> A;
	TArray3<ftype> b;
	TArray3<ftype> x;
	TFdmIccgSolver* LinearSystemSolver;
};