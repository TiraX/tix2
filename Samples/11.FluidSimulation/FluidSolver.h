/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

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
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Vel)
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

private:

private:
	int32 TotalParticles;
	float ParticleMass;
	float ParticleSeperation;
	float RestDenstiy;
	
	float TimeStep;
	float Epsilon;

	aabbox3df BoundaryBox;

	// Param Uniforms
	FPBFParamsPtr PbfParamsUniform;
	FBoundaryInfoPtr BoundaryUniform;

	// Resource Uniforms

	/** Particles with Position and Velocity */
	FParticlesPtr ParticleUniform;

	/** 
	 * Particle Counts in each Cell; 
	 * Size = ParticleCount; 
	 */
	FUniformBufferPtr UB_NumInCell;		

	/**
	 * Particle Indices in each Cell, has up to MaxParticleInCell count particles;
	 * Size = ParticleCount * MaxParticleInCell
	 */
	FUniformBufferPtr UB_CellParticles;

	/**
	 * Accumulated Particle Index Offsets for each Cell
	 * Size = ParticleCount
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
