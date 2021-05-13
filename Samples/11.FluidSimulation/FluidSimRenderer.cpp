/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSimRenderer.h"
#include "FluidSolver.h"
#include "FluidSolverCPU.h"
#include "FluidSolverGPU.h"

#define USE_SOLVER_GPU (0)

bool FFluidSimRenderer::PauseUpdate = false;
bool FFluidSimRenderer::StepNext = false;

FFluidSimRenderer* Instance = nullptr;
FFluidSimRenderer* FFluidSimRenderer::Get()
{
	return Instance;
}

FFluidSimRenderer::FFluidSimRenderer()
	: Solver(nullptr)
{
	Instance = this;
}

FFluidSimRenderer::~FFluidSimRenderer()
{
	MB_Fluid = nullptr;
	PL_Fluid = nullptr;
	ti_delete Solver;

	Instance = nullptr;
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
#if USE_SOLVER_GPU
	Solver = ti_new FFluidSolverGPU;
#else
	Solver = ti_new FFluidSolverCPU;
#endif
	Solver->CreateParticlesInBox(
		aabbox3df(0.2f, 0.2f, 0.2f, 2.6f, 2.6f, 5.0f),
		0.1f, 1.f, 1000.f
	);
	FluidBoundary = aabbox3df(0.f, 0.f, 0.f, 8.f, 3.f, 6.f);
	Solver->SetBoundaryBox(FluidBoundary);
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

void FFluidSimRenderer::MoveBoundary(const vector3df& Offset)
{
	FluidBoundary += Offset;
	Solver->SetBoundaryBox(FluidBoundary);
}

void FFluidSimRenderer::DrawParticles(FRHI* RHI, FScene* Scene)
{
#if USE_SOLVER_GPU
	FFluidSolverGPU* SolverGPU = static_cast<FFluidSolverGPU*>(Solver);
	// Copy simulation result to Mesh Buffer
	RHI->SetResourceStateUB(SolverGPU->GetSimulatedPositions(), RESOURCE_STATE_COPY_SOURCE, false);
	RHI->SetResourceStateMB(MB_Fluid, RESOURCE_STATE_COPY_DEST, false);
	RHI->FlushResourceStateChange();
	RHI->CopyBufferRegion(MB_Fluid, 0, SolverGPU->GetSimulatedPositions(), 0, Solver->GetTotalParticles() * sizeof(vector3df));

	RHI->SetResourceStateMB(MB_Fluid, RESOURCE_STATE_MESHBUFFER, true);
	RHI->SetGraphicsPipeline(PL_Fluid);
	RHI->SetMeshBuffer(MB_Fluid, nullptr);
	RHI->SetUniformBuffer(ESS_VERTEX_SHADER, 0, Scene->GetViewUniformBuffer()->UniformBuffer);

	RHI->DrawPrimitiveInstanced(MB_Fluid, 1, 0);
#else
	FFluidSolverCPU* SolverCPU = static_cast<FFluidSolverCPU*>(Solver);
	FUniformBufferPtr TempPositions = RHI->CreateUniformBuffer(sizeof(vector3df), Solver->GetTotalParticles(), UB_FLAG_INTERMEDIATE);
	TempPositions->SetResourceName("TempPositions");
	RHI->UpdateHardwareResourceUB(TempPositions, SolverCPU->GetSimulatedPositions().data());

	// Copy simulation result to Mesh Buffer
	RHI->SetResourceStateMB(MB_Fluid, RESOURCE_STATE_COPY_DEST, true);
	RHI->CopyBufferRegion(MB_Fluid, 0, TempPositions, 0, Solver->GetTotalParticles() * sizeof(vector3df));

	RHI->SetResourceStateMB(MB_Fluid, RESOURCE_STATE_MESHBUFFER, true);
	RHI->SetGraphicsPipeline(PL_Fluid);
	RHI->SetMeshBuffer(MB_Fluid, nullptr);
	RHI->SetUniformBuffer(ESS_VERTEX_SHADER, 0, Scene->GetViewUniformBuffer()->UniformBuffer);

	RHI->DrawPrimitiveInstanced(MB_Fluid, 1, 0);

#endif
}

static int32 Counter = 0;
void FFluidSimRenderer::Render(FRHI* RHI, FScene* Scene)
{
#if USE_SOLVER_GPU
	//if (!PauseUpdate)
	//{
	//	if (Counter % 10 == 0)
	//	{
	//		PauseUpdate = true;
	//		_LOG(Log, "Pause at %d\n", Counter);
	//	}
	//	++Counter;
	//}
#endif

	Solver->Update(RHI, 1.f / 60);

	RHI->BeginRenderToRenderTarget(RT_BasePass, "BasePass");
	DrawSceneTiles(RHI, Scene);
	DrawParticles(RHI, Scene);

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}