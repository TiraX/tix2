/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSolverGrid2d.h"
#include "FluidSimRenderer.h"


const int32 FFluidSolverGrid2d::SimResolution = 128;
const int32 FFluidSolverGrid2d::DyeResolution = 1024;
const float FFluidSolverGrid2d::DensityDissipation = 1.f;
const float FFluidSolverGrid2d::VelocityDissipation = 0.2f;
const float FFluidSolverGrid2d::Pressure = 0.8f;
const int32 FFluidSolverGrid2d::PressureIterations = 10;
const int32 FFluidSolverGrid2d::Curl = 30;
const float FFluidSolverGrid2d::Radius = 0.25f;
const float FFluidSolverGrid2d::SplatForce = 3000.f;

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
	CreateTextureResource(TexVelocity[0]);
	CreateTextureResource(TexVelocity[1]);
	TextureDesc.Format = EPF_R16F;
	CreateTextureResource(TexDivergence);
	CreateTextureResource(TexCurl);
	CreateTextureResource(TexPressure[0]);
	CreateTextureResource(TexPressure[1]);

	TextureDesc.Format = EPF_RGBA8;
	TextureDesc.Width = DyeResolution;
	TextureDesc.Height = DyeResolution;
	CreateTextureResource(TexDye[0]);
	CreateTextureResource(TexDye[1]);
#undef CreateTextureResource

	UB_Grid2dParam = ti_new FGrid2dParam;
	UB_Grid2dParam->UniformBufferData[0].Info0 = FFloat4(1.f / 60.f, Curl, SimResolution, DyeResolution);
	UB_Grid2dParam->UniformBufferData[0].Info1 = FFloat4(Pressure, VelocityDissipation, 1.f / float(SimResolution), 1.f / float(DyeResolution));
	UB_Grid2dParam->UniformBufferData[0].Info2 = FFloat4(1.f, 0.f, 0.f, DensityDissipation);
	UB_Grid2dParam->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	CurlCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexVelocity[0], TexCurl);
	VorticityCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexCurl, TexVelocity[0], TexVelocity[1]);
	DivergenceCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexVelocity[0], TexDivergence);
	ClearPressureCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexPressure[0], TexPressure[1]);
	PressureCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexDivergence, TexPressure[0], TexPressure[1]);
	GradientSubstractCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexPressure[0], TexVelocity[0], TexVelocity[1]);
	AdvectionVelCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexVelocity[0], TexVelocity[1]);
	AdvectionDyeCS->UpdateComputeParams(RHI, UB_Grid2dParam->UniformBuffer, TexVelocity[0], TexDye[0], TexDye[1]);

	AB_DyeToScreen = RHI->CreateArgumentBuffer(1);
	{
		AB_DyeToScreen->SetTexture(0, TexDye[0]);
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
		MouseMoveDir.X = MouseMoveDelta.X * InvW * SplatForce;
		MouseMoveDir.Y = MouseMoveDelta.Y * InvH * SplatForce;
		const float RadiusScale = 1.f / Radius * 100.f;
		//_LOG(Log, "Mouse: %d %d;  %f %f.\n", CurrMousePos.X, CurrMousePos.Y, MousePointUVVel.X, MousePointUVVel.Y);
		//_LOG(Log, "  Delta: %d %d;  %f %f.\n", MouseMoveDelta.X, MouseMoveDelta.Y, MouseMoveDir.X, MouseMoveDir.Y);
		SplatVelCS->UpdateComputeParams(RHI, 
			UB_Grid2dParam->UniformBuffer, 
			MousePointUVVel, 
			MouseMoveDir, 
			RadiusScale, 
			TexVelocity[0], 
			TexVelocity[1]);
		SplatDyeCS->UpdateComputeParams(RHI, 
			UB_Grid2dParam->UniformBuffer, 
			MousePointUVVel, 
			MouseMoveDir, 
			RadiusScale, 
			TexDye[0],
			TexDye[1]);
	}

	RHI->BeginComputeTask();
	{
		if (MouseMoveDelta != vector2di())
		{
			RHI->BeginEvent("SplatVel");
			RHI->SetResourceStateTexture(TexVelocity[0], RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
			RHI->SetResourceStateTexture(TexVelocity[1], RESOURCE_STATE_UNORDERED_ACCESS, false);
			RHI->FlushResourceStateChange();
			SplatVelCS->Run(RHI);
			RHI->SetResourceStateTexture(TexVelocity[0], RESOURCE_STATE_COPY_DEST, false);
			RHI->SetResourceStateTexture(TexVelocity[1], RESOURCE_STATE_COPY_SOURCE, false);
			RHI->FlushResourceStateChange();
			RHI->CopyTextureRegion(TexVelocity[0], recti(0, 0, SimResolution, SimResolution), 0, TexVelocity[1], 0);
			RHI->EndEvent();

			RHI->BeginEvent("SplatDye");
			RHI->SetResourceStateTexture(TexDye[0], RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
			RHI->SetResourceStateTexture(TexDye[1], RESOURCE_STATE_UNORDERED_ACCESS, false);
			RHI->FlushResourceStateChange();
			SplatDyeCS->Run(RHI);
			RHI->SetResourceStateTexture(TexDye[0], RESOURCE_STATE_COPY_DEST, false);
			RHI->SetResourceStateTexture(TexDye[1], RESOURCE_STATE_COPY_SOURCE, false);
			RHI->FlushResourceStateChange();
			RHI->CopyTextureRegion(TexDye[0], recti(0, 0, DyeResolution, DyeResolution), 0, TexDye[1], 0);
			RHI->EndEvent();
		}

		RHI->BeginEvent("CurlCS");
		RHI->SetResourceStateTexture(TexVelocity[0], RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateTexture(TexCurl, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		CurlCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("VorticityCS");
		RHI->SetResourceStateTexture(TexCurl, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateTexture(TexVelocity[0], RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateTexture(TexVelocity[1], RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		VorticityCS->Run(RHI);
		RHI->SetResourceStateTexture(TexVelocity[0], RESOURCE_STATE_COPY_DEST, false);
		RHI->SetResourceStateTexture(TexVelocity[1], RESOURCE_STATE_COPY_SOURCE, false);
		RHI->FlushResourceStateChange();
		RHI->CopyTextureRegion(TexVelocity[0], recti(0, 0, SimResolution, SimResolution), 0, TexVelocity[1], 0);
		RHI->EndEvent();

		RHI->BeginEvent("DivergenceCS");
		RHI->SetResourceStateTexture(TexVelocity[0], RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateTexture(TexDivergence, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		DivergenceCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("ClearPressureCS");
		RHI->SetResourceStateTexture(TexPressure[0], RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateTexture(TexPressure[1], RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		ClearPressureCS->Run(RHI);
		RHI->SetResourceStateTexture(TexPressure[0], RESOURCE_STATE_COPY_DEST, false);
		RHI->SetResourceStateTexture(TexPressure[1], RESOURCE_STATE_COPY_SOURCE, false);
		RHI->FlushResourceStateChange();
		RHI->CopyTextureRegion(TexPressure[0], recti(0, 0, SimResolution, SimResolution), 0, TexPressure[1], 0);
		RHI->EndEvent();

		RHI->BeginEvent("PressureCS");
		for (int32 i = 0; i < PressureIterations; i++)
		{
			RHI->SetResourceStateTexture(TexDivergence, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
			RHI->SetResourceStateTexture(TexPressure[0], RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
			RHI->SetResourceStateTexture(TexPressure[1], RESOURCE_STATE_UNORDERED_ACCESS, false);
			RHI->FlushResourceStateChange();
			PressureCS->Run(RHI);
			RHI->SetResourceStateTexture(TexPressure[0], RESOURCE_STATE_COPY_DEST, false);
			RHI->SetResourceStateTexture(TexPressure[1], RESOURCE_STATE_COPY_SOURCE, false);
			RHI->FlushResourceStateChange();
			RHI->CopyTextureRegion(TexPressure[0], recti(0, 0, SimResolution, SimResolution), 0, TexPressure[1], 0);
		}
		RHI->EndEvent();

		RHI->BeginEvent("GradientSubstractCS");
		RHI->SetResourceStateTexture(TexPressure[0], RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateTexture(TexVelocity[0], RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateTexture(TexVelocity[1], RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		GradientSubstractCS->Run(RHI);
		RHI->SetResourceStateTexture(TexVelocity[0], RESOURCE_STATE_COPY_DEST, false);
		RHI->SetResourceStateTexture(TexVelocity[1], RESOURCE_STATE_COPY_SOURCE, false);
		RHI->FlushResourceStateChange();
		RHI->CopyTextureRegion(TexVelocity[0], recti(0, 0, SimResolution, SimResolution), 0, TexVelocity[1], 0);
		RHI->EndEvent();

		RHI->BeginEvent("AdvectionVelCS");
		RHI->SetResourceStateTexture(TexVelocity[0], RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateTexture(TexVelocity[1], RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		AdvectionVelCS->Run(RHI);
		RHI->SetResourceStateTexture(TexVelocity[0], RESOURCE_STATE_COPY_DEST, false);
		RHI->SetResourceStateTexture(TexVelocity[1], RESOURCE_STATE_COPY_SOURCE, false);
		RHI->FlushResourceStateChange();
		RHI->CopyTextureRegion(TexVelocity[0], recti(0, 0, SimResolution, SimResolution), 0, TexVelocity[1], 0);
		RHI->EndEvent();

		RHI->BeginEvent("AdvectionDyeCS");
		RHI->SetResourceStateTexture(TexVelocity[0], RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateTexture(TexDye[0], RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateTexture(TexDye[1], RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		AdvectionDyeCS->Run(RHI);
		RHI->SetResourceStateTexture(TexDye[0], RESOURCE_STATE_COPY_DEST, false);
		RHI->SetResourceStateTexture(TexDye[1], RESOURCE_STATE_COPY_SOURCE, false);
		RHI->FlushResourceStateChange();
		RHI->CopyTextureRegion(TexDye[0], recti(0, 0, DyeResolution, DyeResolution), 0, TexDye[1], 0);
		RHI->EndEvent();
	}


	RHI->EndComputeTask();
}

void FFluidSolverGrid2d::RenderGrid(FRHI* RHI, FScene* Scene, FFullScreenRender* FSRenderer)
{
	FSRenderer->DrawFullScreenTexture(RHI, AB_DyeToScreen);
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