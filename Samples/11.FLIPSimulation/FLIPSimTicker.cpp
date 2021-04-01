/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FLIPSimTicker.h"
#include "FLIPSimRenderer.h"

TFLIPSimTicker::TFLIPSimTicker()
{
}

TFLIPSimTicker::~TFLIPSimTicker()
{
}

void TFLIPSimTicker::Tick(float Dt)
{
	TScene * Scene = TEngine::Get()->GetScene();
	Scene->UpdateAllNodesTransforms();
}

void TFLIPSimTicker::SetupScene()
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
