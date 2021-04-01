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
}

void TFlipSolver::TransferFromParticlesToGrids()
{}

void TFlipSolver::ComputeForces()
{}

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