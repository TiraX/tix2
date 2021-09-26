/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FFluidParticle;
class FFluidGrid;

class FFluidSolver
{
public:
	static const int32 MaxParticleInCell = 32 * 32;
	static const int32 MaxNeighbors = 32 * 32;

	FFluidSolver();
	virtual ~FFluidSolver();

	virtual int32 GetNumParticles() { return 0; }

	//void CreateParticlesInBox(
	//	const aabbox3df& InParticleBox,
	//	float InParticleSeperation,
	//	float InParticleMass,
	//	float InRestDenstiy);
	void SetBoundaryBox(const aabbox3df& InCollisionBox);

	void Update(FRHI* RHI, float Dt);
	virtual void Sim(FRHI* RHI, float Dt) = 0;
	//virtual void RenderParticles(FRHI* RHI, FScene* Scene, FMeshBufferPtr Mesh, FPipelinePtr Pipeline) {};
	//virtual void RenderGrid(FRHI* RHI, FScene* Scene, FFullScreenRender* FSRenderer) {};
	//virtual void UpdateMousePosition(const vector2di& InPosition) {};

	//int32 GetTotalParticles() const
	//{
	//	return TotalParticles;
	//}

protected:

protected:
	enum {
		DirtyParams = 1 << 0,
		DirtyBoundary = 1 << 1,
		DirtyParticles = 1 << 2,
	};

	uint32 Flag;
	//int32 TotalParticles;
	//float ParticleMass;
	//float ParticleSeperation;
	//float RestDenstiy;
	//float Viscosity;
	//float Epsilon;
	
	float TimeStep;
	int32 SubStep;

	int32 Iterations;

	//TVector<vector3df> ParticlePositions;

	aabbox3df BoundaryBox;
	//int32 TotalCells;
	//float CellSize;
	//vector3di Dim;
};
