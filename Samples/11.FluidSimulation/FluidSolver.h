/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

BEGIN_UNIFORM_BUFFER_STRUCT(FPBFParams)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, P0)		// x = mass; y = h; z = rho; w = dt
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, P1)		// x = epsilon
END_UNIFORM_BUFFER_STRUCT(FPBFParams)

BEGIN_UNIFORM_BUFFER_STRUCT(FCollisionInfo)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Min)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Max)
END_UNIFORM_BUFFER_STRUCT(FCollisionInfo)

static const int32 MaxParticles = 65536;
BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY_DYNAMIC(FParticles)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Pos)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Vel)
END_UNIFORM_BUFFER_STRUCT(FParticles)

class FFluidSolver
{
public:
	FFluidSolver();
	~FFluidSolver();

	void CreateParticlesInBox(
		const aabbox3df& InParticleBox,
		float InParticleSeperation,
		float InParticleMass,
		float InRestDenstiy);
	void SetCollisionBox(const aabbox3df& InCollisionBox);

private:

private:
	int32 TotalParticles;
	float ParticleMass;
	float ParticleSeperation;
	float RestDenstiy;
	
	float TimeStep;
	float Epsilon;

	aabbox3df CollisionBox;

	FPBFParamsPtr PbfParamsUniform;
	FCollisionInfoPtr CollisionUniform;
	FParticlesPtr ParticleUniform;
};
