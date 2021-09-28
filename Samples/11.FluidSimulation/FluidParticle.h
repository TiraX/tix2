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

	const vector3df& GetParticlePosition(uint32 Index) const
	{
		return ParticlePositions[Index];
	}

	void SetParticlePosition(uint32 Index, const vector3df& P)
	{
		ParticlePositions[Index] = P;
	}

	const TVector<vector3df>& GetParticleVelocities() const
	{
		return ParticleVelocities;
	}

	const vector3df& GetParticleVelocity(uint32 Index) const
	{
		return ParticleVelocities[Index];
	}

	void SetParticleVelocity(uint32 Index, const vector3df& V)
	{
		ParticleVelocities[Index] = V;
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
	TVector<vector3df> ParticleVelocities;
};
