/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TFlipSolver.h"
#include "TMACGrid.h"
#include "TFluidsParticles.h"
#include "TFdmIccgSolver.h"

TFlipSolver::TFlipSolver()
	: Grid(nullptr)
	, Particles(nullptr)
	, LinearSystemSolver(nullptr)
{
	Grid = ti_new TMACGrid;
	Particles = ti_new TFluidsParticles;
}

TFlipSolver::~TFlipSolver()
{
	ti_delete Grid;
	ti_delete Particles;
	ti_delete LinearSystemSolver;
}

void TFlipSolver::InitSolver(const vector3di& InSize, float InSeperation)
{
	Grid->InitSize(InSize, InSeperation);

	A.Resize(InSize);
	b.Resize(InSize);
	x.Resize(InSize);
	LinearSystemSolver = ti_new TFdmIccgSolver(InSize);
}

void TFlipSolver::CreateParticlesInSphere(const vector3df& InCenter, float Radius, float InSeperation)
{
	Particles->InitWithShapeSphere(InCenter, Radius, InSeperation);
}

void TFlipSolver::DoSimulation(float Dt)
{
	TransferFromParticlesToGrids();
	ComputeForces(Dt);
	ComputeViscosity();
	ComputePressure();
	ApplyBoundaryCondition();
	TransferFromGridsToParticles();
	MoveParticles(Dt);

	static int32 Frame = 0;
	{
		int8 Name[128];
		sprintf_s(Name, "pc_%03d.json", Frame++);
		Particles->ExportToJson(Name);
	}
}

void TFlipSolver::TransferFromParticlesToGrids()
{
	Grid->ClearGrids();

	TArray3<float>& U = Grid->U;
	TArray3<float>& V = Grid->V;
	TArray3<float>& W = Grid->W;

	TArray3<float> GridWeights;
	GridWeights.Resize(W.GetSize());
	GridWeights.ResetZero();

	const TVector<TFluidsParticles::TParticle>& P = Particles->Particles;
	TVector<vector3di> GridIndices;
	TVector<float> Weights;
	for (const auto& Particle : P)
	{
		Grid->GetAdjacentGrid(Particle.Position, GridIndices, Weights);

		for (int i = 0 ; i < 8; i++)
		{
			int32 Index = Grid->GetAccessIndex(GridIndices[i]);
			U[Index] += Particle.Velocity.X * Weights[i];
			V[Index] += Particle.Velocity.Y * Weights[i];
			W[Index] += Particle.Velocity.Z * Weights[i];

			GridWeights[Index] += Weights[i];
		}
	}

	const vector3di& Size = GridWeights.GetSize();
	const int32 Count = Size.X * Size.Y * Size.Z;
	for (int32 i = 0; i < Count; i++)
	{
		if (GridWeights[i] > 0.f)
		{
			float InvW = 1.f / GridWeights[i];
			U[i] *= InvW;
			V[i] *= InvW;
			W[i] *= InvW;

			// Mark this grid
			Grid->Markers[i] = TMACGrid::GridFluid;
		}
	}
}

void TFlipSolver::ComputeForces(float Dt)
{
	// Compute gravity only
	const float Gravity = -9.8f;

	TArray3<float>& U = Grid->U;
	TArray3<float>& V = Grid->V;
	TArray3<float>& W = Grid->W;

	const float dv = Gravity * Dt;
	for (auto& w : W.GetData())
	{
		w += dv;
	}
}

void TFlipSolver::ComputeViscosity()
{}

void TFlipSolver::ComputePressure()
{
	BuildLinearSystem();
	LinearSystemSolver->Solve(A, b, x);
	ApplyPressureGradient();
}

void TFlipSolver::ComputeAdvection()
{}
void TFlipSolver::TransferFromGridsToParticles()
{
	const TArray3<float>& U = Grid->U;
	const TArray3<float>& V = Grid->V;
	const TArray3<float>& W = Grid->W;

	TVector<TFluidsParticles::TParticle>& P = Particles->Particles;

	TVector<vector3di> GridIndices;
	TVector<float> Weights;
	for (auto& Particle : P)
	{
		Grid->GetAdjacentGrid(Particle.Position, GridIndices, Weights);
		
		vector3df Vel;
		for (int i = 0; i < 8; i++)
		{
			int32 Index = Grid->GetAccessIndex(GridIndices[i]);
			Vel.X += U[Index] * Weights[i];
			Vel.Y += V[Index] * Weights[i];
			Vel.Z += W[Index] * Weights[i];
		}
		Particle.Velocity = Vel;
	}

}
void TFlipSolver::ExtrapolateVelocityToAir()
{}
void TFlipSolver::ApplyBoundaryCondition()
{
	TI_ASSERT(0);
}
void TFlipSolver::MoveParticles(float Dt)
{
	TVector<TFluidsParticles::TParticle>& P = Particles->Particles;

	for (auto& Particle : P)
	{
		Particle.Position += Particle.Velocity * Dt;
	}
}

void TFlipSolver::BuildLinearSystem()
{
	const vector3di& Size = Grid->Size;

	const float InvH = 1.f / Grid->Seperation;
	const float InvHSq = InvH * InvH;

for (int32 z = 0; z < Size.Z; z++)
{
	for (int32 y = 0; y < Size.Y; y++)
	{
		for (int32 x = 0; x < Size.X; x++)
		{
			int32 Index = Grid->GetAccessIndex(vector3di(x, y, z));

			auto& ARef = A[Index];
			ARef.Center = ARef.Right = ARef.Up = ARef.Front = 0.f;
			b[Index] = 0.f;

			if (Grid->Markers[Index] != TMACGrid::GridSolid)
			{
				b[Index] = Grid->DivergenceAtCellCenter(x, y, z);

				int32 IndexXPlus = Grid->GetAccessIndex(x + 1, y, z);
				if (x + 1 < Size.X && Grid->Markers[IndexXPlus] != TMACGrid::GridSolid)
				{
					ARef.Center += InvHSq;
					if (Grid->Markers[IndexXPlus] == TMACGrid::GridFluid)
					{
						ARef.Right -= InvHSq;
					}
				}

				int32 IndexXNeg = Grid->GetAccessIndex(x - 1, y, z);
				if (x > 0 && Grid->Markers[IndexXNeg] != TMACGrid::GridSolid)
				{
					ARef.Center += InvHSq;
				}

				int32 IndexYPlus = Grid->GetAccessIndex(x, y + 1, z);
				if (y + 1 < Size.Y && Grid->Markers[IndexYPlus] != TMACGrid::GridSolid)
				{
					ARef.Center += InvHSq;
					if (Grid->Markers[IndexYPlus] == TMACGrid::GridFluid)
					{
						ARef.Up -= InvHSq;
					}
				}

				int32 IndexYNeg = Grid->GetAccessIndex(x, y - 1, z);
				if (y > 0 && Grid->Markers[IndexYNeg] != TMACGrid::GridSolid)
				{
					ARef.Center += InvHSq;
				}

				int32 IndexZPlus = Grid->GetAccessIndex(x, y, z + 1);
				if (z + 1 < Size.Z && Grid->Markers[IndexZPlus] != TMACGrid::GridSolid)
				{
					ARef.Center += InvHSq;
					if (Grid->Markers[IndexZPlus] == TMACGrid::GridFluid)
					{
						ARef.Front -= InvHSq;
					}
				}

				int32 IndexZNeg = Grid->GetAccessIndex(x, y, z - 1);
				if (z > 0 && Grid->Markers[IndexZNeg] != TMACGrid::GridSolid)
				{
					ARef.Center += InvHSq;
				}
			}
			else
			{
				// Not GridFluid
				ARef.Center = 1.f;
			}
		}
	}
}
}

void TFlipSolver::ApplyPressureGradient()
{
	const vector3di& Size = Grid->Size;
	const float InvH = 1.f / Grid->Seperation;

	TArray3<float>& U = Grid->U;
	TArray3<float>& V = Grid->V;
	TArray3<float>& W = Grid->W;

	for (int32 k = 0; k < Size.Z; k++)
	{
		for (int32 j = 0; j < Size.Y; j++)
		{
			for (int32 i = 0; i < Size.X; i++)
			{
				int32 Index = Grid->GetAccessIndex(i, j, k);
				int32 IndexX1 = Grid->GetAccessIndex(i + 1, j, k);
				int32 IndexY1 = Grid->GetAccessIndex(i, j + 1, k);
				int32 IndexZ1 = Grid->GetAccessIndex(i, j, k + 1);
				if (Grid->Markers[Index] == TMACGrid::GridFluid)
				{
					if (i + 1 < Size.X && Grid->Markers[IndexX1] != TMACGrid::GridSolid)
					{
						U[IndexX1] = U[IndexX1] + InvH * (x[IndexX1] - x[Index]);
					}
					if (j + 1 < Size.Y && Grid->Markers[IndexY1] != TMACGrid::GridSolid)
					{
						V[IndexY1] = V[IndexY1] + InvH * (x[IndexY1] - x[Index]);
					}
					if (k + 1 < Size.Z && Grid->Markers[IndexZ1] != TMACGrid::GridSolid)
					{
						W[IndexZ1] = W[IndexZ1] + InvH * (x[IndexZ1] - x[Index]);
					}
				}
			}
		}
	}
}


