/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSimTicker.h"
#include "FluidSimRenderer.h"

TFluidSimTicker::TFluidSimTicker()
	: Action(Action_None)
{
}

TFluidSimTicker::~TFluidSimTicker()
{
}

void TFluidSimTicker::Tick(float Dt)
{
	const float Speed = 1.f;
	if (Action == Action_MoveLeft)
	{
		float delta = -Speed * Dt;
		ENQUEUE_RENDER_COMMAND(MoveBoundBox)(
			[delta]()
			{
				FFluidSimRenderer::Get()->MoveBoundary(vector3df(delta, 0.f, 0.f));
			});

	}
	else if (Action == Action_MoveRight)
	{
		float delta = Speed * Dt;
		ENQUEUE_RENDER_COMMAND(MoveBoundBox)(
			[delta]()
			{
				FFluidSimRenderer::Get()->MoveBoundary(vector3df(delta, 0.f, 0.f));
			});
	}
}

void TFluidSimTicker::SetupScene()
{
	FVTSystem::SetVTEnabled(false);

	// PreLoad default material first
	const TString DefaultMaterial = "M_Debug.tasset";
	TAssetLibrary::Get()->LoadAsset(DefaultMaterial);
	const TString ParticleMaterial = "M_Particle.tasset";
	TAssetLibrary::Get()->LoadAsset(ParticleMaterial);

	// Load scene
	const TString TargetSceneAsset = "Map_OceanFFT.tasset";
	//TEngine::Get()->GetScene()->LoadSceneAync(TargetSceneAsset);

	// Setup Camera
	TEngine::Get()->GetScene()->GetActiveCamera()->SetPosition(vector3df(4.4f, 16.f, 8.f));
	TEngine::Get()->GetScene()->GetActiveCamera()->SetTarget(vector3df(4.4f, 0.5f, 1.f));
}

bool TFluidSimTicker::OnEvent(const TEvent& e)
{
	if (e.type == EET_KEY_DOWN)
	{
		if (e.param == KEY_SPACE)
		{
			// pause / resume simulation
			ENQUEUE_RENDER_COMMAND(PauseResumeSim)(
				[]()
				{
					FFluidSimRenderer::PauseUpdate = !FFluidSimRenderer::PauseUpdate;
				});
		}
		else if (e.param == KEY_KEY_P)
		{
			// step next sim
			ENQUEUE_RENDER_COMMAND(NextSim)(
				[]()
				{
					FFluidSimRenderer::StepNext = true;
				});
		}
		else if (e.param == KEY_LEFT)
		{
			Action = Action_MoveLeft;
		}
		else if (e.param == KEY_RIGHT)
		{
			Action = Action_MoveRight;
		}
	} 
	else if (e.type == EET_KEY_UP)
	{
		Action = Action_None;
	}
	return true;
}