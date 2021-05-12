/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GTAOTicker.h"
#include "GTAORenderer.h"

TGTAOTicker::TGTAOTicker()
{
}

TGTAOTicker::~TGTAOTicker()
{
}

void TGTAOTicker::Tick(float Dt)
{
}

void TGTAOTicker::SetupScene()
{
	FVTSystem::SetVTEnabled(false);

	// PreLoad default material first
	const TString DefaultMaterial = "M_Debug.tasset";
	const TString DefaultMaterialInstance = "DebugMaterial.tasset";
	TAssetLibrary::Get()->LoadAsset(DefaultMaterial);
	TAssetLibrary::Get()->LoadAsset(DefaultMaterialInstance);

	// Load scene
	const TString TargetSceneAsset = "Room.tasset";
	//const TString TargetSceneAsset = "Slum01.tasset";
	TEngine::Get()->GetScene()->LoadSceneAync(TargetSceneAsset);
}
