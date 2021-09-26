/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FluidSolver.h"

class FFluidParticle
{
public:
	FFluidParticle();
	~FFluidParticle();

	void CreateParticlesInBox(
		const aabbox3df& InParticleBox,
		float InParticleSeperation,
		float InParticleMass);

	int32 GetTotalParticles() const
	{
		return (int32)ParticlePositions.size();
	}

	const TVector<vector3df>& GetParticlePositions() const
	{
		return ParticlePositions;
	}

	float GetParticleMass() const
	{
		return ParticleMass;
	}

	float GetParticleSeperation() const
	{
		return ParticleSeperation;
	}
private:
private:
	float ParticleMass;
	float ParticleSeperation;

	TVector<vector3df> ParticlePositions;
};
