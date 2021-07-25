/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSolverGrid2d.h"
#include "FluidSimRenderer.h"


const int32 FFluidSolverGrid2d::SimResolution = 128;
const int32 FFluidSolverGrid2d::DyeResolution = 512;
const float FFluidSolverGrid2d::DensityDissipation = 1.f;
const float FFluidSolverGrid2d::VelocityDissipation = 0.2f;
const float FFluidSolverGrid2d::Pressure = 0.8f;
const int32 FFluidSolverGrid2d::PressureIterations = 20;
const int32 FFluidSolverGrid2d::Curl = 30;
const float FFluidSolverGrid2d::Radius = 0.25f;

FFluidSolverGrid2d::FFluidSolverGrid2d()
{
	SubStep = 1;
	
	// Compute tasks
	FRHI* RHI = FRHI::Get();
#define INIT_CS(cs) cs = ti_new F##cs;\
	cs->Finalize();\
	cs->PrepareResources(RHI)

	INIT_CS(CurlCS);
	INIT_CS(VorticityCS);
	INIT_CS(DivergenceCS);
	INIT_CS(ClearPressureCS);
	INIT_CS(PressureCS);
	INIT_CS(GradientSubstractCS);
	INIT_CS(AdvectionVelCS);
	INIT_CS(AdvectionDyeCS);
	INIT_CS(SplatVelCS);
	INIT_CS(SplatDyeCS);
#undef INIT_CS
}

FFluidSolverGrid2d::~FFluidSolverGrid2d()
{
}

void FFluidSolverGrid2d::CreateGrid(FRHI* RHI, FFullScreenRender* FSRender)
{
	TTextureDesc TextureDesc;
	TextureDesc.Format = EPF_RG16F;
	TextureDesc.AddressMode = ETC_CLAMP_TO_EDGE;

#define CreateTextureResource(T) T=RHI->CreateTexture(TextureDesc); \
	T->SetTextureFlag(ETF_UAV, true); \
	T->SetResourceName(#T); \
	RHI->UpdateHardwareResourceTexture(T)

	TextureDesc.Width = SimResolution;
	TextureDesc.Height = SimResolution;
	CreateTextureResource(TexVelocity);
	TextureDesc.Format = EPF_R16F;
	CreateTextureResource(TexDivergence);
	CreateTextureResource(TexCurl);
	CreateTextureResource(TexPressure);

	TextureDesc.Format = EPF_RGBA8;
	TextureDesc.Width = DyeResolution;
	TextureDesc.Height = DyeResolution;
	CreateTextureResource(TexDye);
#undef CreateTextureResource

	UB_Grid2dParam = ti_new FGrid2dParam;
	UB_Grid2dParam->UniformBufferData[0].Info0 = FFloat4(1.f / 60.f, Curl, SimResolution, DyeResolution);
	UB_Grid2dParam->UniformBufferData[0].Info1 = FFloat4(Pressure, VelocityDissipation, 1.f / float(SimResolution), 1.f / float(DyeResolution));
	UB_Grid2dParam->UniformBufferData[0].Info2 = FFloat4(1.f, 0.f, 0.f, DensityDissipation);
	UB_Grid2dParam->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	CurlCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexVelocity, TexCurl);
	VorticityCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexCurl, TexVelocity);
	DivergenceCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexVelocity, TexDivergence);
	ClearPressureCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexPressure);
	PressureCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexDivergence, TexPressure);
	GradientSubstractCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexPressure, TexVelocity);
	AdvectionVelCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexVelocity);
	AdvectionDyeCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexVelocity, TexDye);

	AB_DyeToScreen = RHI->CreateArgumentBuffer(1);
	{
		AB_DyeToScreen->SetTexture(0, TexDye);
		RHI->UpdateHardwareResourceAB(AB_DyeToScreen, FSRender->GetFullScreenShader(), 0);
	}
}

void FFluidSolverGrid2d::Sim(FRHI * RHI, float Dt)
{
	// Update splat parameters
	{
		vector2df MousePointUVVel, MouseMoveDir;
		const float InvW = 1.f / RHI->GetViewport().Width;
		const float InvH = 1.f / RHI->GetViewport().Height;
		MousePointUVVel.X = CurrMousePos.X * InvW;
		MousePointUVVel.Y = CurrMousePos.Y * InvH;
		MouseMoveDir.X = MouseMoveDelta.X * InvW;
		MouseMoveDir.Y = MouseMoveDelta.Y * InvH;
		const float RadiusScale = 1.f / Radius * 100.f;
		//_LOG(Log, "Mouse: %d %d;  %f %f.\n", CurrMousePos.X, CurrMousePos.Y, MousePointUVVel.X, MousePointUVVel.Y);
		//_LOG(Log, "  Delta: %d %d;  %f %f.\n", MouseMoveDelta.X, MouseMoveDelta.Y, MouseMoveDir.X, MouseMoveDir.Y);
		SplatVelCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, MousePointUVVel, MouseMoveDir, RadiusScale, TexVelocity);
		SplatDyeCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, MousePointUVVel, MouseMoveDir, RadiusScale, TexDye);
	}

	RHI->BeginComputeTask();
	{
		//if (MouseMoveDelta != vector2di())
		{
			RHI->BeginEvent("SplatVel");
			SplatVelCS->Run(RHI);
			RHI->EndEvent();

			RHI->BeginEvent("SplatDye");
			SplatDyeCS->Run(RHI);
			RHI->EndEvent();
		}

		RHI->BeginEvent("CurlCS");
		CurlCS->Run(RHI);
		RHI->EndEvent();

		//RHI->BeginEvent("VorticityCS");
		//VorticityCS->Run(RHI);
		//RHI->EndEvent();

		//RHI->BeginEvent("DivergenceCS");
		//DivergenceCS->Run(RHI);
		//RHI->EndEvent();

		//RHI->BeginEvent("ClearPressureCS");
		//ClearPressureCS->Run(RHI);
		//RHI->EndEvent();

		//RHI->BeginEvent("PressureCS");
		//for (int32 i = 0; i < PressureIterations; i++)
		//{
		//	PressureCS->Run(RHI);
		//}
		//RHI->EndEvent();

		//RHI->BeginEvent("GradientSubstractCS");
		//GradientSubstractCS->Run(RHI);
		//RHI->EndEvent();

		//RHI->BeginEvent("AdvectionVelCS");
		//AdvectionVelCS->Run(RHI);
		//RHI->EndEvent();

		//RHI->BeginEvent("AdvectionDyeCS");
		//AdvectionDyeCS->Run(RHI);
		//RHI->EndEvent();
	}


	RHI->EndComputeTask();
}

void FFluidSolverGrid2d::RenderGrid(FRHI* RHI, FScene* Scene, FFullScreenRender* FSRenderer)
{
	//FSRenderer->DrawFullScreenTexture(RHI, AB_DyeToScreen);
}

void FFluidSolverGrid2d::UpdateMousePosition(const vector2di& InPosition)
{
	if (LastMousePos == vector2di())
	{
		LastMousePos = InPosition;
		return;
	}
	else
	{
		CurrMousePos = InPosition;
	}

	MouseMoveDelta = CurrMousePos - LastMousePos;
	LastMousePos = CurrMousePos;
}