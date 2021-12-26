/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "RTAOTicker.h"
#include "RTAORenderer.h"

TRTAOTicker::TRTAOTicker()
{
}

TRTAOTicker::~TRTAOTicker()
{
}

void TRTAOTicker::Tick(float Dt)
{
}

void TRTAOTicker::SetupScene()
{
	FVTSystem::SetVTEnabled(false);

	// PreLoad default material first
	const TString DefaultMaterial = "M_Debug.tasset";
	const TString DefaultMaterialInstance = "DebugMaterial.tasset";
	TAssetLibrary::Get()->LoadAsset(DefaultMaterial);
	TAssetLibrary::Get()->LoadAsset(DefaultMaterialInstance);

	// Preload rtx pipeline
	const TString RTAOPipeline = "RTX_AO.tasset";
	TAssetLibrary::Get()->LoadAsset(RTAOPipeline);

	// Load scene
	const TString TargetSceneAsset = "ArchVis_RT.tasset";
	//const TString TargetSceneAsset = "Slum01.tasset";
	TEngine::Get()->GetScene()->LoadSceneAync(TargetSceneAsset);
}
