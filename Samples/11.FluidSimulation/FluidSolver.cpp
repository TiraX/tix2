/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSolver.h"

FFluidSolver::FFluidSolver()
	: TotalParticles(0)
	, ParticleMass(1.f)
	, ParticleSeperation(0.1f)
	, RestDenstiy(1000.f)
	, TimeStep(1.f / 60.f / 2.f)
	, Epsilon(600.f)
{
	PbfParamsUniform = ti_new FPBFParams;
	BoundaryUniform = ti_new FBoundaryInfo;
}

FFluidSolver::~FFluidSolver()
{
}

void FFluidSolver::CreateParticlesInBox(
	const aabbox3df& InParticleBox,
	float InParticleSeperation,
	float InParticleMass,
	float InRestDenstiy)
{
	// Calc Total Particles
	vector3df Ext = InParticleBox.getExtent();
	vector3di Dim;
	Dim.X = (int32)(Ext.X / ParticleSeperation);
	Dim.Y = (int32)(Ext.Y / ParticleSeperation);
	Dim.Z = (int32)(Ext.Z / ParticleSeperation);
	TotalParticles = Dim.X * Dim.Y * Dim.Z;

	// Update params and uniforms
	ParticleMass = InParticleMass;
	ParticleSeperation = InParticleSeperation;
	RestDenstiy = InRestDenstiy;
	TI_ASSERT(0);	// Create Uniforms

	// Boundary Uniforms
	TI_ASSERT(0);

	// Create particles and resources

	ParticleUniform = ti_new FParticles(TotalParticles);
	int32 Index = 0;
	const float jitter = ParticleSeperation * 0.5f;
	TMath::RandSeed(12306);
	for (int32 z = 0; z < Dim.Z; z++)
	{
		for (int32 y = 0; y < Dim.Y; y++)
		{
			for (int32 x = 0; x < Dim.X; x++)
			{
				FFloat4& Pos = ParticleUniform->UniformBufferData[Index].Pos;
				Pos.X = x * ParticleSeperation + InParticleBox.MinEdge.X + TMath::RandomUnit() * jitter;
				Pos.Y = y * ParticleSeperation + InParticleBox.MinEdge.Y + TMath::RandomUnit() * jitter;
				Pos.Z = z * ParticleSeperation + InParticleBox.MinEdge.Z + TMath::RandomUnit() * jitter;
				++Index;
			}
		}
	}
	ParticleUniform->InitUniformBuffer(UB_FLAG_COMPUTE_WRITABLE);

	// Other Uniforms
	TI_ASSERT(0);
	TI_TODO("Split pos and vel to 2 buffers");
	TI_TODO("Create Sorted Position buffer");
}

void FFluidSolver::SetBoundaryBox(const aabbox3df& InBoundaryBox)
{
	BoundaryBox = InBoundaryBox;
}