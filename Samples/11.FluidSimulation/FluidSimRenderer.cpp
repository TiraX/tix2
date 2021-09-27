/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSimRenderer.h"
#include "FluidParticle.h"
#include "FluidGrid.h"
#include "FluidSolver.h"
#include "FluidSolverPbfCPU.h"
#include "FluidSolverPbfGPU.h"
#include "FluidSolverGrid2d.h"
#include "FluidSolverFlipCPU.h"
#include "ParticleRenderer.h"

#define SOLVER_PBF_CPU (0)
#define SOLVER_PBF_GPU (1)
#define SOLVER_GRID2D (2)
#define SOLVER_FLIP_CPU (3)

#define FLUID_SOLVER SOLVER_FLIP_CPU

bool FFluidSimRenderer::PauseUpdate = false;
bool FFluidSimRenderer::StepNext = false;

FFluidSimRenderer* Instance = nullptr;
FFluidSimRenderer* FFluidSimRenderer::Get()
{
	return Instance;
}

FFluidSimRenderer::FFluidSimRenderer()
	: ParticleRenderer(nullptr)
	, Solver(nullptr)
{
	Instance = this;
}

FFluidSimRenderer::~FFluidSimRenderer()
{
	MB_Fluid = nullptr;
	PL_Fluid = nullptr;
	ti_delete Solver;

	ti_delete ParticleRenderer;

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
#if FLUID_SOLVER == SOLVER_PBF_CPU
	FFluidSolverPbfCPU* SolverLocal = ti_new FFluidSolverPbfCPU;
#elif FLUID_SOLVER == SOLVER_PBF_GPU
	FFluidSolverPbfGPU* SolverLocal = ti_new FFluidSolverPbfGPU;
#elif FLUID_SOLVER == SOLVER_GRID2D
	FFluidSolverGrid2d* SolverLocal = ti_new FFluidSolverGrid2d;
#elif FLUID_SOLVER == SOLVER_FLIP_CPU
	FFluidSolverFlipCPU* SolverLocal = ti_new FFluidSolverFlipCPU;
#else
	TI_ASSERT(0);
#endif
	Solver = SolverLocal;

	const float ParticleSeperation = 0.1f;
	const float ParticleMass = 1.f;
	FluidBoundary = aabbox3df(0.f, 0.f, 0.f, 8.f, 3.f, 6.f);
	const aabbox3df ParticleBox(0.2f, 0.2f, 0.2f, 2.6f, 2.6f, 5.0f);
	const vector3di GridDimension = vector3di(8, 3, 6) * 5;

#if FLUID_SOLVER == SOLVER_GRID2D
	SolverLocal->CreateGrid(RHI, &FSRender);
#elif FLUID_SOLVER == SOLVER_FLIP_CPU
	Solver->SetBoundaryBox(FluidBoundary);
	SolverLocal->CreateParticles(ParticleBox, ParticleSeperation, ParticleMass);
	SolverLocal->CreateGrid(GridDimension);
	ParticleRenderer = ti_new FParticleRenderer();
	ParticleRenderer->CreateResources(RHI, Solver->GetNumParticles(), FluidBoundary);
#else
	// SOLVER_PBF_CPU & SOLVER_PBF_GPU
	Solver->SetBoundaryBox(FluidBoundary);
	SolverLocal->CreateParticles(ParticleBox, ParticleSeperation, ParticleMass);
	SolverLocal->CreateNeighborSearchGrid(FluidBoundary.getExtent(), ParticleSeperation);

	ParticleRenderer = ti_new FParticleRenderer();
	ParticleRenderer->CreateResources(RHI, Solver->GetNumParticles(), FluidBoundary);
#endif
}

void FFluidSimRenderer::MoveBoundary(const vector3df& Offset)
{
	FluidBoundary += Offset;
	Solver->SetBoundaryBox(FluidBoundary);
}

static int32 Counter = 0;
void FFluidSimRenderer::Render(FRHI* RHI, FScene* Scene)
{
#if FLUID_SOLVER == SOLVER_GRID2D
	FFluidSolverGrid2d* SolverGrid2D = (FFluidSolverGrid2d*)Solver;
	SolverGrid2D->UpdateMousePosition(MousePosition);
#endif

	Solver->Update(RHI, 1.f / 60);

	RHI->BeginRenderToRenderTarget(RT_BasePass, "BasePass");
	DrawSceneTiles(RHI, Scene);

#if FLUID_SOLVER == SOLVER_PBF_CPU
	FFluidSolverPbfCPU* SolverPbfCPU = (FFluidSolverPbfCPU*)Solver;
	ParticleRenderer->UploadParticles(RHI, SolverPbfCPU->GetSimulatedPositions());
#elif FLUID_SOLVER == SOLVER_PBF_GPU
	FFluidSolverPbfGPU* SolverPbfGPU = (FFluidSolverPbfGPU*)Solver;
	ParticleRenderer->UploadParticles(RHI, SolverPbfGPU->GetSimulatedPositions());
#elif FLUID_SOLVER == SOLVER_FLIP_CPU
	FFluidSolverFlipCPU* SolverFlipCPU = (FFluidSolverFlipCPU*)Solver;
	ParticleRenderer->UploadParticles(RHI, SolverFlipCPU->GetSimulatedPositions());
#endif
	if (ParticleRenderer != nullptr)
		ParticleRenderer->DrawParticles(RHI, Scene);

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
#if FLUID_SOLVER == SOLVER_GRID2D
	FFluidSolverGrid2d* SolverGrid2d = (FFluidSolverGrid2d*)Solver;
	SolverGrid2d->RenderGrid(RHI, Scene, &FSRender);
#endif
}