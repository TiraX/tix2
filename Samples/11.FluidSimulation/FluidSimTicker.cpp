/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSimTicker.h"
#include "FluidSimRenderer.h"

TFluidSimTicker::TFluidSimTicker()
{
}

TFluidSimTicker::~TFluidSimTicker()
{
}

void TFluidSimTicker::Tick(float Dt)
{
	TScene * Scene = TEngine::Get()->GetScene();
	Scene->UpdateAllNodesTransforms();
}

void TFluidSimTicker::SetupScene()
{
	FVTSystem::SetVTEnabled(false);

	// PreLoad default material first
	const TString DefaultMaterial = "M_Debug.tasset";
	const TString DefaultMaterialInstance = "DebugMaterial.tasset";
	TAssetLibrary::Get()->LoadAsset(DefaultMaterial);
	//TAssetLibrary::Get()->LoadAsset(DefaultMaterialInstance);

	// Load scene
	const TString TargetSceneAsset = "Map_OceanFFT.tasset";
	//TEngine::Get()->GetScene()->LoadSceneAync(TargetSceneAsset);
}
