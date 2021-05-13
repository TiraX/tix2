/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSolverCPU.h"
#include "FluidSimRenderer.h"

#define RUN_PARALLEL (1)

FFluidSolverCPU::FFluidSolverCPU()
{
}

FFluidSolverCPU::~FFluidSolverCPU()
{
}

void FFluidSolverCPU::UpdateParamBuffers(FRHI* RHI)
{
}

void FFluidSolverCPU::UpdateResourceBuffers(FRHI * RHI)
{
	/** Particles with Position and Velocity */
	if ((Flag & DirtyParticles) != 0)
	{
		TI_ASSERT(ParticlePositions.size() == TotalParticles);
		UB_ParticlePositions = ParticlePositions;
	}
	if (UB_ParticleVelocities.size() != TotalParticles)
	{
		UB_ParticleVelocities.resize(TotalParticles);
		UB_SortedPositions.resize(TotalParticles);
		UB_SortedVelocities.resize(TotalParticles);
	}

	// Cells
	if (UB_NumInCell.size() != TotalCells)
	{
		UB_NumInCell.resize(TotalCells);
		UB_CellParticles.resize(TotalCells * MaxParticleInCell);
		UB_CellParticleOffsets.resize(TotalCells);
	}

	// Particles
	if (UB_PositionOld.size() != TotalParticles)
	{
		UB_PositionOld.resize(TotalParticles);
		UB_NeighborNum.resize(TotalParticles);
		UB_NeighborParticles.resize(TotalParticles * MaxNeighbors);
		UB_Lambdas.resize(TotalParticles);
		UB_DeltaPositions.resize(TotalParticles);
	}
}

#define BRUTE_FORCE_NB_SEARCH (0)
void FFluidSolverCPU::Sim(FRHI * RHI, float Dt)
{
	// Update uniform buffers
	static int32 CPUFrame = 0;

	if (Flag != 0)
	{
		UpdateParamBuffers(RHI);
		UpdateResourceBuffers(RHI);
		Flag = 0;
	}

#if BRUTE_FORCE_NB_SEARCH
	UB_SortedPositions = UB_ParticlePositions;
	UB_SortedVelocities = UB_ParticleVelocities;
	ApplyGravity();
	NeighborSearchBF();
#else
	CellInit();
	P2Cell();
	CalcOffset();
	Sort();
	ApplyGravity();
	NeighborSearch();
#endif
	for (int32 iter = 0; iter < Iterations; ++iter)
	{
		Lambda();
		DeltaPos();
		ApplyDeltaPos();
	}
	UpdateVelocity();

	// Copy sorted positions, velocities to particle positions and velocities
	if (!FFluidSimRenderer::PauseUpdate)
	{
		UB_ParticlePositions.swap(UB_SortedPositions);
		UB_ParticleVelocities.swap(UB_SortedVelocities);
		_LOG(Log, "CPU Sim - %d.\n", CPUFrame++);
	}
	else
	{
		// Pause state change, always simulate the previous data
		if (FFluidSimRenderer::StepNext)
		{
			// Step next data
			UB_ParticlePositions.swap(UB_SortedPositions);
			UB_ParticleVelocities.swap(UB_SortedVelocities);

			_LOG(Log, "CPU Sim - %d.\n", CPUFrame++);
			FFluidSimRenderer::StepNext = false;
		}
	}
}

void FFluidSolverCPU::CellInit()
{
	memset(UB_NumInCell.data(), 0, UB_NumInCell.size() * sizeof(uint32));
	memset(UB_CellParticleOffsets.data(), 0, UB_CellParticleOffsets.size() * sizeof(uint32));
}
inline uint32 GetCellHash(const vector3di& Index, const vector3di& Dim)
{
	return (Index.Z * Dim.Y + Index.Y) * Dim.X + Index.X;
}
inline vector3di GetCellIndex3(const vector3df& Pos, const vector3df& BMin, float CellSizeInv)
{
	vector3df _CellIndex3 = (Pos - BMin) * CellSizeInv;
	vector3di CellIndex3;
	CellIndex3.X = (int32)_CellIndex3.X;
	CellIndex3.Y = (int32)_CellIndex3.Y;
	CellIndex3.Z = (int32)_CellIndex3.Z;

	return CellIndex3;
}
void FFluidSolverCPU::P2Cell()
{
	const float CellSizeInv = 1.f / CellSize;
	for (int32 p = 0; p < TotalParticles; p++)
	{
		vector3df Pos = UB_ParticlePositions[p];

		vector3di CellIndex3 = GetCellIndex3(Pos, BoundaryBox.MinEdge, CellSizeInv);
		uint32 CellIndex = GetCellHash(CellIndex3, Dim);

		const uint32 Num = UB_NumInCell[CellIndex];
		//if (Num < MaxParticleInCell)
		TI_ASSERT(Num < MaxParticleInCell);
		{
			UB_CellParticles[CellIndex * MaxParticleInCell + Num] = p;
			++UB_NumInCell[CellIndex];
		}
	}
}
void FFluidSolverCPU::CalcOffset()
{
	int32 TotalCells = Dim.X * Dim.Y * Dim.Z;
	UB_CellParticleOffsets[0] = 0;
	for (int i = 1; i < TotalCells; i++)
	{
		UB_CellParticleOffsets[i] = UB_CellParticleOffsets[i - 1] + UB_NumInCell[i - 1];
	}
}
void FFluidSolverCPU::Sort()
{
	int32 TotalCells = Dim.X * Dim.Y * Dim.Z;
#if RUN_PARALLEL
#pragma omp parallel for
#endif
	for (int32 c = 0; c < TotalCells; c++)
	{
		const uint32 Num = UB_NumInCell[c];
		const uint32 Offset = UB_CellParticleOffsets[c];
		for (uint32 p = 0; p < Num; p++)
		{
			uint32 SrcIndex = UB_CellParticles[c * MaxParticleInCell + p];
			uint32 DstIndex = Offset + p;

			UB_SortedPositions[DstIndex] = UB_ParticlePositions[SrcIndex];
			UB_SortedVelocities[DstIndex] = UB_ParticleVelocities[SrcIndex];
		}
	}
}

inline void BoundaryCheck(vector3df& Pos, const vector3df& BMin, const vector3df& BMax)
{
	const float epsilon = 1e-5f;

	if (Pos.X <= BMin.X)
		Pos.X = BMin.X + epsilon;

	if (Pos.Y <= BMin.Y)
		Pos.Y = BMin.Y + epsilon;

	if (Pos.Z <= BMin.Z)
		Pos.Z = BMin.Z + epsilon;

	if (Pos.X >= BMax.X)
		Pos.X = BMax.X - epsilon;

	if (Pos.Y >= BMax.Y)
		Pos.Y = BMax.Y - epsilon;

	if (Pos.Z >= BMax.Z)
		Pos.Z = BMax.Z - epsilon;
}
void FFluidSolverCPU::ApplyGravity()
{
	const vector3df Gravity = vector3df(0.f, 0.f, -9.8f);
	const float Dt = (TimeStep / SubStep);
	const vector3df GravityDt = Gravity * Dt;

#if RUN_PARALLEL
#pragma omp parallel for
#endif
	for (int32 p = 0; p < TotalParticles; p++)
	{
		vector3df Pos = UB_SortedPositions[p];
		vector3df Vel = UB_SortedVelocities[p];

		UB_PositionOld[p] = Pos;

		Vel += GravityDt;
		Pos += Vel * Dt;

		BoundaryCheck(Pos, BoundaryBox.MinEdge, BoundaryBox.MaxEdge);

		UB_SortedPositions[p] = Pos;
	}

}
void FFluidSolverCPU::NeighborSearch()
{
	const float h = ParticleSeperation;
	const float h2 = h * h;
	const float h3_inv = 1.f / (h * h * h);

	const vector3di CellIndexOffsets[27] =
	{
		vector3di(-1,-1,-1),
		vector3di(-1,-1, 0),
		vector3di(-1,-1, 1),
		vector3di(-1, 0,-1),
		vector3di(-1, 0, 0),
		vector3di(-1, 0, 1),
		vector3di(-1, 1,-1),
		vector3di(-1, 1, 0),
		vector3di(-1, 1, 1),

		vector3di(0,-1,-1),
		vector3di(0,-1, 0),
		vector3di(0,-1, 1),
		vector3di(0, 0,-1),
		vector3di(0, 0, 0),
		vector3di(0, 0, 1),
		vector3di(0, 1,-1),
		vector3di(0, 1, 0),
		vector3di(0, 1, 1),

		vector3di(1,-1,-1),
		vector3di(1,-1, 0),
		vector3di(1,-1, 1),
		vector3di(1, 0,-1),
		vector3di(1, 0, 0),
		vector3di(1, 0, 1),
		vector3di(1, 1,-1),
		vector3di(1, 1, 0),
		vector3di(1, 1, 1),
	};

	const float CellSizeInv = 1.f / CellSize;
#if RUN_PARALLEL
#pragma omp parallel for
#endif
	for (int32 p = 0; p < TotalParticles; p++)
	{
		vector3df Pos = UB_SortedPositions[p];

		int32 NumNb = 0;
		vector3di CellIndex3 = GetCellIndex3(Pos, BoundaryBox.MinEdge, CellSizeInv);

		const uint32 NbParticleOffset = p * MaxNeighbors;
		for (int cell = 0; cell < 27; ++cell)
		{
			vector3di NbCellIndex3 = CellIndex3 + CellIndexOffsets[cell];
			if (NbCellIndex3.X >= 0 && NbCellIndex3.X < Dim.X &&
				NbCellIndex3.Y >= 0 && NbCellIndex3.Y < Dim.Y &&
				NbCellIndex3.Z >= 0 && NbCellIndex3.Z < Dim.Z)
			{
				uint32 NbCellIndex = GetCellHash(NbCellIndex3, Dim);
				uint32 NumNeighborsInCell = UB_NumInCell[NbCellIndex];
				uint32 NbParticleCellOffset = UB_CellParticleOffsets[NbCellIndex];

				for (uint32 i = 0; i < NumNeighborsInCell; i++)
				{
					uint32 NbIndex = NbParticleCellOffset + i;
					if (NbIndex != p)
					{
						vector3df NbPos = UB_SortedPositions[NbIndex];
						vector3df Dir = Pos - NbPos;
						float LengthSq = Dir.dotProduct(Dir);
						if (LengthSq < h2)
						{
							UB_NeighborParticles[NbParticleOffset + NumNb] = NbIndex;
							NumNb++;
							TI_ASSERT(NumNb <= MaxNeighbors);
						}
					}
				}
			}
		}
		UB_NeighborNum[p] = NumNb;
	}
}

void FFluidSolverCPU::NeighborSearchBF()
{
	const float h = ParticleSeperation;
	const float h2 = h * h;
	const float h3_inv = 1.f / (h * h * h);

#if RUN_PARALLEL
#pragma omp parallel for
#endif
	for (int32 p = 0; p < TotalParticles; p++)
	{
		vector3df Pos = UB_SortedPositions[p];

		int32 NumNb = 0;
		for (int32 i = 0; i < TotalParticles; i++)
		{
			if (i == p)
				continue;

			const vector3df& NbPos = UB_SortedPositions[i];
			float lsq = (NbPos - Pos).getLengthSQ();
			if (lsq < h2)
			{
				UB_NeighborParticles[p * MaxNeighbors + NumNb] = i;
				++NumNb;
			}
		}
		UB_NeighborNum[p] = NumNb;
	}
}

static const float Poly6Factor = 315.f / 64.f / PI;
inline float poly6_value(float s, float h, float h2, float h3_inv)
{
	float result = 0.f;
	if (s < h)  // Try to ignore this if
	{
		float x = (h2 - s * s) * h3_inv;
		result = Poly6Factor * x * x * x;
	}
	return result;
}

static const float SpikyGradFactor = -45.f / PI;
inline vector3df spiky_gradient(const vector3df& Dir, float s, float h, float h3_inv)
{
	vector3df result = vector3df(0, 0, 0);
	if (s < h)  // Try to ignore this if
	{
		float x = (h - s) * h3_inv;
		float g_factor = SpikyGradFactor * x * x;
		result = Dir * g_factor;
	}
	return result;
}
void FFluidSolverCPU::Lambda()
{
	const float h = ParticleSeperation;
	const float h2 = h * h;
	const float h3_inv = 1.f / (h * h * h);
	const float m_by_rho = ParticleMass / RestDenstiy;
#if RUN_PARALLEL
#pragma omp parallel for
#endif
	for (int32 p = 0; p < TotalParticles; p++)
	{
		const int32 Index = p;
		const uint32 NumNb = UB_NeighborNum[Index];
		const uint32 NbParticleOffset = Index * MaxNeighbors;
		const vector3df Pos = UB_SortedPositions[Index];

		vector3df Grad = vector3df(0, 0, 0);
		float SumGradSq = 0.f;
		float Density = 0.f;
		for (uint32 i = 0; i < NumNb; i++)
		{
			uint32 NbIndex = UB_NeighborParticles[NbParticleOffset + i];
			vector3df NbPos = UB_SortedPositions[NbIndex];

			vector3df Dir = Pos - NbPos;
			float s = TMath::Max(Dir.getLength(), 1e-5f);
			Dir /= s;
			vector3df NbGrad = spiky_gradient(Dir, s, h, h3_inv) * m_by_rho;
			Grad += NbGrad;
			SumGradSq += NbGrad.dotProduct(NbGrad);
			Density += poly6_value(s, h, h2, h3_inv);
		}

		SumGradSq += Grad.dotProduct(Grad);
		float DensityContraint = max(Density * m_by_rho, 0.f);
		UB_Lambdas[Index] = (-DensityContraint) / (SumGradSq + Epsilon);
	}
}
void FFluidSolverCPU::DeltaPos()
{
	const float h = ParticleSeperation;
	const float h2 = h * h;
	const float h3_inv = 1.f / (h * h * h);
	const float m_by_rho = ParticleMass / RestDenstiy;
#if RUN_PARALLEL
#pragma omp parallel for
#endif
	for (int32 p = 0; p < TotalParticles; p++)
	{
		const int32 Index = p;
		const uint32 NumNb = UB_NeighborNum[Index];
		const uint32 NbParticleOffset = Index * MaxNeighbors;
		const vector3df Pos = UB_SortedPositions[Index];

		const float Lambda = UB_Lambdas[Index];

		vector3df DeltaPos = vector3df(0, 0, 0);
		for (uint32 i = 0; i < NumNb; i++)
		{
			int NbIndex = UB_NeighborParticles[NbParticleOffset + i];

			float NbLambda = UB_Lambdas[NbIndex];

			vector3df NbPos = UB_SortedPositions[NbIndex];
			vector3df Dir = Pos - NbPos;
			float s = TMath::Max(Dir.getLength(), 1e-5f);
			Dir /= s;

			DeltaPos += spiky_gradient(Dir, s, h, h3_inv) * (Lambda + NbLambda);
		}

		DeltaPos *= m_by_rho;
		UB_DeltaPositions[Index] = DeltaPos;
	}

}
void FFluidSolverCPU::ApplyDeltaPos()
{
#if RUN_PARALLEL
#pragma omp parallel for
#endif
	for (int32 p = 0; p < TotalParticles; p++)
	{
		UB_SortedPositions[p] += UB_DeltaPositions[p];
	}
}
void FFluidSolverCPU::UpdateVelocity()
{
	const float DtInv = 1.f / (TimeStep / SubStep);
#if RUN_PARALLEL
#pragma omp parallel for
#endif
	for (int32 p = 0; p < TotalParticles; p++)
	{
		vector3df Pos = UB_SortedPositions[p];
		vector3df Vel = UB_SortedVelocities[p];

		BoundaryCheck(Pos, BoundaryBox.MinEdge, BoundaryBox.MaxEdge);

		Vel = (Pos - UB_PositionOld[p]) * DtInv;

		UB_SortedPositions[p] = Pos;
		UB_SortedVelocities[p] = Vel;
	}
}