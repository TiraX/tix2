/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "CellInitCS.h"
#include "P2CellCS.h"
#include "CalcOffsetsCS.h"
#include "SortCS.h"
#include "ApplyGravityCS.h"
#include "NeighborSearchCS.h"
#include "LambdaCS.h"
#include "DeltaPosCS.h"
#include "UpdateVelocityCS.h"

BEGIN_UNIFORM_BUFFER_STRUCT(FPBFParams)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, P0)		// x = mass; y = epsilon; z = m/rho; w = dt
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, P1)		// x = h; y = h^2; z = 1.f/(h^3) w = inv_cell_size;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FInt4, Dim)		// xyz = Dim, w = TotalParticles
END_UNIFORM_BUFFER_STRUCT(FPBFParams)

BEGIN_UNIFORM_BUFFER_STRUCT(FBoundaryInfo)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Min)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Max)
END_UNIFORM_BUFFER_STRUCT(FBoundaryInfo)

BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY_DYNAMIC(FParticles)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Pos)
END_UNIFORM_BUFFER_STRUCT(FParticles)

class FFluidSolver
{
public:
	static const int32 MaxParticleInCell = 32;
	static const int32 MaxNeighbors = 32;

	FFluidSolver();
	~FFluidSolver();

	void CreateParticlesInBox(
		const aabbox3df& InParticleBox,
		float InParticleSeperation,
		float InParticleMass,
		float InRestDenstiy);
	void SetBoundaryBox(const aabbox3df& InCollisionBox);

	void Update(FRHI* RHI, float Dt);

	int32 GetTotalParticles() const
	{
		return TotalParticles;
	}

	FUniformBufferPtr GetSimulatedPositions()
	{
		return UB_ParticlePositions;
	}

private:
	void UpdateParamBuffers(FRHI* RHI);
	void UpdateResourceBuffers(FRHI * RHI);
	void UpdateComputeParams(FRHI * RHI);

private:
	enum {
		DirtyParams = 1 << 0,
		DirtyBoundary = 1 << 1,
		DirtyParticles = 1 << 2,
	};

	uint32 Flag;
	int32 TotalParticles;
	float ParticleMass;
	float ParticleSeperation;
	float RestDenstiy;
	
	float TimeStep;
	float Epsilon;

	int32 Iterations;

	TVector<vector3df> ParticlePositions;

	aabbox3df BoundaryBox;
	int32 TotalCells;
	float CellSize;
	vector3di Dim;

	// Compute Tasks
	FCellInitCSPtr CellInitCS;
	FP2CellCSPtr P2CellCS;
	FCalcOffsetsCSPtr CalcOffsetsCS;
	FSortCSPtr SortCS;
	FApplyGravityCSPtr ApplyGravityCS;
	FNeighborSearchCSPtr NeighborSearchCS;
	FLambdaCSPtr LambdaCS;
	FDeltaPosCSPtr DeltaPosCS;
	FUpdateVelocityCSPtr UpdateVelocityCS;

	// Param Uniforms
	FPBFParamsPtr UB_PbfParams;
	FBoundaryInfoPtr UB_Boundary;

	// Resource Uniforms

	/** Particles with Position and Velocity */
	FUniformBufferPtr UB_ParticlePositions;
	FUniformBufferPtr UB_ParticleVelocities;
	FUniformBufferPtr UB_SortedPositions;
	FUniformBufferPtr UB_SortedVelocities;

	/** 
	 * Particle Counts in each Cell; 
	 * Size = CellCount; 
	 */
	FUniformBufferPtr UB_NumInCell;		

	/**
	 * Particle Indices in each Cell, has up to MaxParticleInCell count particles;
	 * Size = CellCount * MaxParticleInCell
	 */
	FUniformBufferPtr UB_CellParticles;

	/**
	 * Accumulated Particle Index Offsets for each Cell
	 * Size = CellCount
	 */
	FUniformBufferPtr UB_CellParticleOffsets;

	/**
	 * Position backup Buffer
	 * Size = ParticleCount
	 */
	FUniformBufferPtr UB_PositionOld;

	/**
	 * Particle Neighbor Counts for each particle;
	 * Size = ParticleCount;
	 */
	FUniformBufferPtr UB_NeighborNum;

	/**
	 * Particle Neighbor Indices for each particle, has up to MaxNeighbors count particles;
	 * Size = ParticleCount * MaxNeighbors
	 */
	FUniformBufferPtr UB_NeighborParticles;

	/**
	 * Lambdas from density for each particle 
	 * Size = ParticleCount
	 */
	FUniformBufferPtr UB_Lambdas;
};
