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
	TScene* Scene = TEngine::Get()->GetScene();

	// Send camera info to render thread
	TNodeCamera* Cam = Scene->GetActiveCamera();
	if ((Cam->GetCameraFlags() & TNodeCamera::ECAMF_MAT_VIEW_UPDATED) != 0)
	{
		vector3df Pos = Cam->GetAbsolutePosition();
		vector3df Dir = Cam->GetCamDir();
		vector3df Hor = Cam->GetHorVector();
		vector3df Ver = Cam->GetVerVector();
		ENQUEUE_RENDER_COMMAND(UpdateCamInfoRenderThread)(
			[Pos, Dir, Hor, Ver]()
			{
				FRTAORenderer::Get()->UpdateCamInfo(Pos, Dir, Hor, Ver);
			});
	}
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
	const TString PathtracerPipeline = "RTX_Pathtracer.tasset";
	TAssetLibrary::Get()->LoadAsset(PathtracerPipeline);

	// Load scene
	const TString TargetSceneAsset = "ArchVis_RT.tasset";
	//const TString TargetSceneAsset = "Slum01.tasset";
	TEngine::Get()->GetScene()->LoadSceneAync(TargetSceneAsset);
}
