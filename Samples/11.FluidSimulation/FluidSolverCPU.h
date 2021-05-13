/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FluidSolver.h"

class FFluidSolverCPU : public FFluidSolver
{
public:
	FFluidSolverCPU();
	virtual ~FFluidSolverCPU();

	virtual void Sim(FRHI* RHI, float Dt) override;

	const TVector<vector3df>& GetSimulatedPositions()
	{
		return UB_ParticlePositions;
	}

private:
	void UpdateParamBuffers(FRHI* RHI);
	void UpdateResourceBuffers(FRHI* RHI);

	void CellInit();
	void P2Cell();
	void CalcOffset();
	void Sort();
	void ApplyGravity();
	void NeighborSearch();
	void NeighborSearchBF();
	void Lambda();
	void DeltaPos();
	void ApplyDeltaPos();
	void UpdateVelocity();

private:
	// Resource Uniforms
	/** Particles with Position and Velocity */ 
	TVector<vector3df> UB_ParticlePositions;
	TVector<vector3df> UB_ParticleVelocities;
	TVector<vector3df> UB_SortedPositions;
	TVector<vector3df> UB_SortedVelocities;

	/** 
	 * Particle Counts in each Cell; 
	 * Size = CellCount; 
	 */
	TVector<uint32> UB_NumInCell;

	/**
	 * Particle Indices in each Cell, has up to MaxParticleInCell count particles;
	 * Size = CellCount * MaxParticleInCell
	 */
	TVector<uint32> UB_CellParticles;

	/**
	 * Accumulated Particle Index Offsets for each Cell
	 * Size = CellCount
	 */
	TVector<uint32> UB_CellParticleOffsets;

	/**
	 * Position backup Buffer
	 * Size = ParticleCount
	 */
	TVector<vector3df> UB_PositionOld;

	/**
	 * Particle Neighbor Counts for each particle;
	 * Size = ParticleCount;
	 */
	TVector<uint32> UB_NeighborNum;

	/**
	 * Particle Neighbor Indices for each particle, has up to MaxNeighbors count particles;
	 * Size = ParticleCount * MaxNeighbors
	 */
	TVector<uint32> UB_NeighborParticles;

	/**
	 * Lambdas from density for each particle 
	 * Size = ParticleCount
	 */
	TVector<float> UB_Lambdas;

	/**
	 * Delta Pos for each particle
	 * Size = ParticleCount
	 */
	TVector<vector3df> UB_DeltaPositions;
};
