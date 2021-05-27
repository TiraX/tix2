/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SkinnedAnimationTicker.h"
#include "SkinnedAnimationRenderer.h"
#include "Player.h"

TSkinnedAnimationTicker::TSkinnedAnimationTicker()
	: MainPlayer(nullptr)
{
}

TSkinnedAnimationTicker::~TSkinnedAnimationTicker()
{
}

void TSkinnedAnimationTicker::Tick(float Dt)
{
}

bool TSkinnedAnimationTicker::OnEvent(const TEvent& e)
{
	if (e.type == EET_KEY_DOWN)
	{
		if (e.param == KEY_SPACE)
		{
			TEngine::Get()->FreezeTick();
		}
		else if (e.param == KEY_KEY_P)
		{
			TEngine::Get()->TickStepNext();
		}	
	}
	else if (e.type == EET_KEY_UP)
	{
	}
	return true;
}

void TSkinnedAnimationTicker::SetupScene()
{
	FVTSystem::SetVTEnabled(false);

	// PreLoad default material first
	const TString DefaultMaterial = "M_Debug.tasset";
	TAssetLibrary::Get()->LoadAsset(DefaultMaterial);
	const TString DefaultSKMaterial = "M_DebugSkinMesh.tasset";
	TAssetLibrary::Get()->LoadAsset(DefaultSKMaterial);

	// Load scene
	const TString TargetSceneAsset = "Map_Anim.tasset";
	TEngine::Get()->GetScene()->LoadSceneAync(TargetSceneAsset);
}

void TSkinnedAnimationTicker::CreatePlayer()
{
	MainPlayer = ti_new Player();
	MainPlayer->LoadResources();
}
