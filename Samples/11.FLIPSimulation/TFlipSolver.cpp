/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TFlipSolver.h"
#include "TMACGrid.h"
#include "TFluidsParticles.h"

TFlipSolver::TFlipSolver()
{
	Grid = ti_new TMACGrid;
	Particles = ti_new TFluidsParticles;
}

TFlipSolver::~TFlipSolver()
{
	ti_delete Grid;
	ti_delete Particles;
}

void TFlipSolver::InitGrid(const vector3di& InSize, float InSeperation)
{
	Grid->InitSize(InSize, InSeperation);
}

void TFlipSolver::CreateParticlesInSphere(const vector3df& InCenter, float Radius, float InSeperation)
{
	Particles->InitWithShapeSphere(InCenter, Radius, InSeperation);
}

void TFlipSolver::DoSimulation(float Dt)
{
	TransferFromParticlesToGrids();
	ComputeForces();
	ComputeViscosity();
	ComputePressure();
	ComputeAdvection();
	TransferFromGridsToParticles();
	//ExtrapolateVelocityToAir();
	//ApplyBoundaryCondition();
	MoveParticles();

	static int32 Frame = 0;
	{
		int8 Name[128];
		sprintf_s(Name, "pc_%03d.json", Frame++);
		Particles->ExportToJson(Name);
	}
}

void TFlipSolver::TransferFromParticlesToGrids()
{
	Grid->ClearVelocities();

	TVector<float>& U = Grid->U;
	TVector<float>& V = Grid->V;
	TVector<float>& W = Grid->W;

	TVector<float> GridWeights;
	GridWeights.resize(W.size());
	memset(W.data(), 0, W.size() * sizeof(float));

	const TVector<TFluidsParticles::TParticle>& P = Particles->Particles;
	TVector<vector3di> GridIndices;
	TVector<float> Weights;
	for (const auto& Particle : P)
	{
		Grid->GetAdjacentGrid(Particle.Position, GridIndices, Weights);

		for (int i = 0 ; i < 8; i++)
		{
			int32 Index = Grid->GetAccessIndex(GridIndices[i]);
			U[Index] += Particle.Velocity.X;
			V[Index] += Particle.Velocity.Y;
			W[Index] += Particle.Velocity.Z;

			GridWeights[Index] += Weights[i];
		}
	}
	const int Count = (int)U.size();
	for (int i = 0; i < Count; i++)
	{
		if (GridWeights[i] > 0.f)
		{
			float InvW = 1.f / GridWeights[i];
			U[i] *= InvW;
			V[i] *= InvW;
			W[i] *= InvW;
		}
	}
}

void TFlipSolver::ComputeForces()
{
}

void TFlipSolver::ComputeViscosity()
{}


void TFlipSolver::ComputePressure()
{}
void TFlipSolver::ComputeAdvection()
{}
void TFlipSolver::TransferFromGridsToParticles()
{}
void TFlipSolver::ExtrapolateVelocityToAir()
{}
void TFlipSolver::ApplyBoundaryCondition()
{}
void TFlipSolver::MoveParticles()
{}