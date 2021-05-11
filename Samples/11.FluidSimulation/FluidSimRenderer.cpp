/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSimRenderer.h"
#include "FluidSolver.h"

FFluidSimRenderer::FFluidSimRenderer()
	: Solver(nullptr)
{
}

FFluidSimRenderer::~FFluidSimRenderer()
{
	MB_Fluid = nullptr;
	PL_Fluid = nullptr;
	ti_delete Solver;
}

void FFluidSimRenderer::InitRenderFrame(FScene* Scene)
{
	// Prepare frame view uniform buffer
	Scene->InitRenderFrame();
}

void FFluidSimRenderer::InitInRenderThread()
{
	FDefaultRenderer::InitInRenderThread();

	FRHI * RHI = FRHI::Get();
	FSRender.InitCommonResources(RHI);

	const int32 RTWidth = 1600;
	const int32 RTHeight = TEngine::AppInfo.Height * RTWidth / TEngine::AppInfo.Width;

	TStreamPtr ArgumentValues = ti_new TStream;
	TVector<FTexturePtr> ArgumentTextures;

	// Setup base pass render target
	RT_BasePass = FRenderTarget::Create(RTWidth, RTHeight);
	RT_BasePass->SetResourceName("RT_BasePass");
	RT_BasePass->AddColorBuffer(EPF_RGBA16F, 1, ERTC_COLOR0, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_BasePass->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8, 1, ERT_LOAD_CLEAR, ERT_STORE_DONTCARE);
	RT_BasePass->Compile();

	AB_Result = RHI->CreateArgumentBuffer(1);
	{
		AB_Result->SetTexture(0, RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture);
		RHI->UpdateHardwareResourceAB(AB_Result, FSRender.GetFullScreenShader(), 0);
	}

	// Init Simulation
	Solver = ti_new FFluidSolver;
	Solver->CreateParticlesInBox(
		aabbox3df(0.2f, 0.2f, 0.2f, 2.6f, 2.6f, 5.0f),
		0.1f, 1.f, 1000.f
	);
	const aabbox3df FluidBoundary(0.f, 0.f, 0.f, 8.f, 3.f, 6.f);
	Solver->SetBoundaryBox(
		FluidBoundary
	);
	// Create render resources
	MB_Fluid = RHI->CreateEmptyMeshBuffer(EPT_POINTLIST, EVSSEG_POSITION, Solver->GetTotalParticles(), EIT_16BIT, 0, FluidBoundary);
	MB_Fluid->SetResourceName("MB_Fluid");
	RHI->UpdateHardwareResourceMesh(MB_Fluid, Solver->GetTotalParticles() * sizeof(vector3df), sizeof(vector3df), 0, EIT_32BIT, "MB_Fluid");

	// Load default pipeline
	const TString ParticleMaterialName = "M_Particle.tasset";
	TAssetPtr ParticleMaterialAsset = TAssetLibrary::Get()->LoadAsset(ParticleMaterialName);
	TResourcePtr ParticleMaterialResource = ParticleMaterialAsset->GetResourcePtr();
	TMaterialPtr ParticleMaterial = static_cast<TMaterial*>(ParticleMaterialResource.get());
	PL_Fluid = ParticleMaterial->PipelineResource;
}

void FFluidSimRenderer::DrawParticles(FRHI* RHI, FScene* Scene)
{
	// Copy simulation result to Mesh Buffer
	RHI->SetResourceStateUB(Solver->GetSimulatedPositions(), RESOURCE_STATE_COPY_SOURCE, false);
	RHI->SetResourceStateMB(MB_Fluid, RESOURCE_STATE_COPY_DEST, false);
	RHI->FlushResourceStateChange();
	RHI->CopyBufferRegion(MB_Fluid, 0, Solver->GetSimulatedPositions(), 0, Solver->GetTotalParticles() * sizeof(vector3df));

	RHI->SetResourceStateMB(MB_Fluid, RESOURCE_STATE_MESHBUFFER, true);
	RHI->SetGraphicsPipeline(PL_Fluid);
	RHI->SetMeshBuffer(MB_Fluid, nullptr);
	RHI->SetUniformBuffer(ESS_VERTEX_SHADER, 0, Scene->GetViewUniformBuffer()->UniformBuffer);
	//ApplyShaderParameter(RHI, Scene, OceanPrimitive);
	//RHI->SetRenderResourceTable(2, OceanDisplacementResource);
	//RHI->SetRenderResourceTable(3, OceanNormalResource);

	RHI->DrawPrimitiveInstanced(
		MB_Fluid,
		1,
		0);
}

void FFluidSimRenderer::Render(FRHI* RHI, FScene* Scene)
{
	Solver->Update(RHI, 1.f / 60);

	RHI->BeginRenderToRenderTarget(RT_BasePass, "BasePass");
	DrawSceneTiles(RHI, Scene);
	DrawParticles(RHI, Scene);

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}