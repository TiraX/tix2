/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FFluidSolver
{
public:
	static const int32 MaxParticleInCell = 256;
	static const int32 MaxNeighbors = 32;

	FFluidSolver();
	virtual ~FFluidSolver();

	void CreateParticlesInBox(
		const aabbox3df& InParticleBox,
		float InParticleSeperation,
		float InParticleMass,
		float InRestDenstiy);
	void SetBoundaryBox(const aabbox3df& InCollisionBox);

	virtual void Update(FRHI* RHI, float Dt) = 0;

	int32 GetTotalParticles() const
	{
		return TotalParticles;
	}

protected:

protected:
	enum {
		DirtyParams = 1 << 0,
		DirtyBoundary = 1 << 1,
		DirtyParticles = 1 << 2,
	};

	uint32 Flag;
	int32 TotalParticles;
	float ParticleMass;
	float ParticleSeperation;
	float RestDenstiy;
	
	float TimeStep;
	float Epsilon;

	int32 Iterations;

	TVector<vector3df> ParticlePositions;

	aabbox3df BoundaryBox;
	int32 TotalCells;
	float CellSize;
	vector3di Dim;
};
