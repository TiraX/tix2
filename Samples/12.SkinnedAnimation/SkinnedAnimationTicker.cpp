/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SkinnedAnimationTicker.h"
#include "SkinnedAnimationRenderer.h"

TSkinnedAnimationTicker::TSkinnedAnimationTicker()
{
}

TSkinnedAnimationTicker::~TSkinnedAnimationTicker()
{
}

void TSkinnedAnimationTicker::Tick(float Dt)
{
}

void TSkinnedAnimationTicker::SetupScene()
{
	FVTSystem::SetVTEnabled(false);

	// PreLoad default material first
	const TString DefaultMaterial = "M_Debug.tasset";
	TAssetLibrary::Get()->LoadAsset(DefaultMaterial);

	// Load scene
	const TString TargetSceneAsset = "Map_Anim.tasset";
	TEngine::Get()->GetScene()->LoadSceneAync(TargetSceneAsset);
}
