/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ParticleRenderer.h"

FParticleRenderer::FParticleRenderer()
{
}

FParticleRenderer::~FParticleRenderer()
{
	MB_Fluid = nullptr;
	PL_Fluid = nullptr;
}

void FParticleRenderer::CreateResources(FRHI* RHI, int32 NumParticles, const aabbox3df& BBox)
{
	// Create render resources
	MB_Fluid = RHI->CreateEmptyMeshBuffer(EPT_POINTLIST, EVSSEG_POSITION, NumParticles, EIT_16BIT, 0, BBox);
	MB_Fluid->SetResourceName("MB_Fluid");
	RHI->UpdateHardwareResourceMesh(MB_Fluid, NumParticles * sizeof(vector3df), sizeof(vector3df), 0, EIT_32BIT, "MB_Fluid");

	// Load default pipeline
	const TString ParticleMaterialName = "M_Particle.tasset";
	TAssetPtr ParticleMaterialAsset = TAssetLibrary::Get()->LoadAsset(ParticleMaterialName);
	TResourcePtr ParticleMaterialResource = ParticleMaterialAsset->GetResourcePtr();
	TMaterialPtr ParticleMaterial = static_cast<TMaterial*>(ParticleMaterialResource.get());
	PL_Fluid = ParticleMaterial->PipelineResource;
}

void FParticleRenderer::UploadParticles(FRHI* RHI, const TVector<vector3df>& ParticlePositions)
{
	FUniformBufferPtr TempPositions = RHI->CreateUniformBuffer(sizeof(vector3df), (uint32)ParticlePositions.size(), UB_FLAG_INTERMEDIATE);
	TempPositions->SetResourceName("TempPositions");
	RHI->UpdateHardwareResourceUB(TempPositions, ParticlePositions.data());
	
	// Copy simulation result to Mesh Buffer
	RHI->SetResourceStateMB(MB_Fluid, RESOURCE_STATE_COPY_DEST, true);
	RHI->CopyBufferRegion(MB_Fluid, 0, TempPositions, 0, (uint32)ParticlePositions.size() * sizeof(vector3df));
}

void FParticleRenderer::DrawParticles(FRHI* RHI, FScene* Scene)
{
	RHI->SetResourceStateMB(MB_Fluid, RESOURCE_STATE_MESHBUFFER, true);
	RHI->SetGraphicsPipeline(PL_Fluid);
	RHI->SetMeshBuffer(MB_Fluid, nullptr);
	RHI->SetUniformBuffer(ESS_VERTEX_SHADER, 0, Scene->GetViewUniformBuffer()->UniformBuffer);

	RHI->DrawPrimitiveInstanced(MB_Fluid, 1, 0);
}