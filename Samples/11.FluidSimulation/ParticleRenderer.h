/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FParticleRenderer
{
public:
	static bool PauseUpdate;
	static bool StepNext;

	FParticleRenderer();
	~FParticleRenderer();

	void CreateResources(FRHI* RHI, int32 NumParticles, const aabbox3df& BBox);
	void UploadParticles(FRHI* RHI, const TVector<vector3df>& ParticlePositions);

	void DrawParticles(FRHI* RHI, FScene* Scene);
private:

private:
	FMeshBufferPtr MB_Fluid;
	FPipelinePtr PL_Fluid;
};