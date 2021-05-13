/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSolver.h"
#include "FluidSimRenderer.h"

FFluidSolver::FFluidSolver()
	: Flag(0)
	, TotalParticles(0)
	, ParticleMass(1.f)
	, ParticleSeperation(0.1f)
	, RestDenstiy(1000.f)
	, TimeStep(1.f / 60.f / 2.f)
	, Epsilon(600.f)
	, Iterations(3)
	, TotalCells(0)
	, CellSize(1.f)
{
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
	const float dis = ParticleSeperation * 0.9f;
	vector3df Ext = InParticleBox.getExtent();
	vector3di ParticleDim;
	ParticleDim.X = (int32)(Ext.X / dis);
	ParticleDim.Y = (int32)(Ext.Y / dis);
	ParticleDim.Z = (int32)(Ext.Z / dis);
	TotalParticles = ParticleDim.X * ParticleDim.Y * ParticleDim.Z;

	// Update params
	ParticleMass = InParticleMass;
	ParticleSeperation = InParticleSeperation;
	RestDenstiy = InRestDenstiy;
	Flag |= DirtyParams;


	// Create particles and resources
	ParticlePositions.reserve(TotalParticles);
	const float jitter = 0.f;// ParticleSeperation * 0.5f;
	TMath::RandSeed(12306);
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
	TotalParticles = (int32)ParticlePositions.size();
	Flag |= DirtyParticles;
}

void FFluidSolver::SetBoundaryBox(const aabbox3df& InBoundaryBox)
{
	BoundaryBox = InBoundaryBox;
	CellSize = ParticleSeperation;
	const float CellSizeInv = 1.f / CellSize;
	
	vector3df BoundExt = BoundaryBox.getExtent();
	Dim.X = (int32)(BoundExt.X * CellSizeInv + 1);
	Dim.Y = (int32)(BoundExt.Y * CellSizeInv + 1);
	Dim.Z = (int32)(BoundExt.Z * CellSizeInv + 1);
	
	TotalCells = Dim.X * Dim.Y * Dim.Z;

	Flag |= DirtyBoundary;
}
