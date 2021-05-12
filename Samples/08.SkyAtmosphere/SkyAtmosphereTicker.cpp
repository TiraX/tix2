/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SkyAtmosphereTicker.h"
#include "SkyAtmosphereRenderer.h"

TSkyAtmosphereTicker::TSkyAtmosphereTicker()
{
}

TSkyAtmosphereTicker::~TSkyAtmosphereTicker()
{
}

void TSkyAtmosphereTicker::Tick(float Dt)
{
}

void TSkyAtmosphereTicker::SetupScene()
{
	FVTSystem::SetVTEnabled(false);

	// PreLoad default material first
	const TString DefaultMaterial = "M_Debug.tasset";
	const TString DefaultMaterialInstance = "DebugMaterial.tasset";
	TAssetLibrary::Get()->LoadAsset(DefaultMaterial);
	TAssetLibrary::Get()->LoadAsset(DefaultMaterialInstance);

	// PreLoad sky material
	const TString SkyMaterial = "M_Sky.tasset";
	TAssetLibrary::Get()->LoadAsset(SkyMaterial);

	// Load scene
	const TString TargetSceneAsset = "SkyAtmosphereMap.tasset";
	//const TString TargetSceneAsset = "Slum01.tasset";
	TEngine::Get()->GetScene()->LoadSceneAync(TargetSceneAsset);
}
