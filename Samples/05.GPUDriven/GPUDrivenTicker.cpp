/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GPUDrivenTicker.h"

TGPUDrivenTicker::TGPUDrivenTicker()
{
}

TGPUDrivenTicker::~TGPUDrivenTicker()
{
}

void TGPUDrivenTicker::Tick(float Dt)
{
	TScene * Scene = TEngine::Get()->GetScene();
	Scene->UpdateAllNodesTransforms();
}

void TGPUDrivenTicker::SetupScene()
{
	// Load scene
	const TString TargetSceneAsset = "Slum01.tasset";
	TEngine::Get()->GetScene()->LoadSceneAync(TargetSceneAsset);
}
