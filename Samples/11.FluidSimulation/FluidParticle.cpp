/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidParticle.h"


FFluidParticle::FFluidParticle()
	: ParticleMass(1.f)
	, ParticleSeperation(0.1f)
{
}

FFluidParticle::~FFluidParticle()
{
}

void FFluidParticle::CreateParticlesInBox(
	const aabbox3df& InParticleBox,
	float InParticleSeperation,
	float InParticleMass)
{
	// Calc Total Particles
	ParticleSeperation = InParticleSeperation;
	const float dis = ParticleSeperation;
	vector3df Ext = InParticleBox.getExtent();
	vector3di ParticleDim;
	ParticleDim.X = (int32)(Ext.X / dis);
	ParticleDim.Y = (int32)(Ext.Y / dis);
	ParticleDim.Z = (int32)(Ext.Z / dis);
	int32 TotalParticles = ParticleDim.X * ParticleDim.Y * ParticleDim.Z;

	// Update params
	ParticleMass = InParticleMass;
	ParticleSeperation = InParticleSeperation;


	// Create particles and resources
	ParticlePositions.reserve(TotalParticles);
	const float jitter = ParticleSeperation * 0.9f;
	TMath::RandSeed(12306);
#define USE_DEBUG_POINTS (0)
#if USE_DEBUG_POINTS
	TFile F;
	if (F.Open("debug_particles.dat", EFA_READ))
	{
		uint32 Num;
		F.Read(&Num, sizeof(uint32), sizeof(uint32));
		ParticlePositions.resize(Num);
		F.Read(ParticlePositions.data(), Num * sizeof(vector3df), Num * sizeof(vector3df));
		F.Close();
	}
#else
	for (float z = InParticleBox.MinEdge.Z; z < InParticleBox.MaxEdge.Z; z += dis)
	{
		for (float y = InParticleBox.MinEdge.Y; y < InParticleBox.MaxEdge.Y; y += dis)
		{
			for (float x = InParticleBox.MinEdge.X; x < InParticleBox.MaxEdge.X; x += dis)
			{
				vector3df Pos;
				Pos.X = x + TMath::RandomUnit() * jitter;
				Pos.Y = y + TMath::RandomUnit() * jitter;
				Pos.Z = z + TMath::RandomUnit() * jitter;
				ParticlePositions.push_back(Pos);
			}
		}
	}
#endif
	TotalParticles = (int32)ParticlePositions.size();

	// Create Velocity and init to Zero
	ParticleVelocities.resize(TotalParticles);
}