/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GPUDrivenTicker.h"
#include "GPUDrivenRenderer.h"

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

	// Send frustum info to render thread
	TNodeCamera * Cam = Scene->GetActiveCamera();
	if ((Cam->GetCameraFlags() & TNodeCamera::ECAMF_MAT_VIEW_UPDATED) != 0)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(UpdateFrustumRenderThread,
			SViewFrustum, Frustum, Cam->GetFrustum(),
			{
				FGPUDrivenRenderer::Get()->UpdateFrustumUniform(Frustum);
			});
	}
}

void TGPUDrivenTicker::SetupScene()
{
	FVTSystem::SetVTEnabled(false);

	// PreLoad default material first
	const TString DefaultMaterial = "M_Debug.tasset";
	const TString DefaultMaterialInstance = "DebugMaterial.tasset";
	TAssetLibrary::Get()->LoadAsset(DefaultMaterial);
	TAssetLibrary::Get()->LoadAsset(DefaultMaterialInstance);

	// PreLoad depth only material first
	const TString DepthOnlyMaterial = "M_DepthOnly.tasset";
	const TString DepthOnlyMaterialInstance = "DepthOnlyMaterial.tasset";
	TAssetLibrary::Get()->LoadAsset(DepthOnlyMaterial);
	TAssetLibrary::Get()->LoadAsset(DepthOnlyMaterialInstance);

	// Load scene
	//const TString TargetSceneAsset = "CubeTest01.tasset";
	const TString TargetSceneAsset = "Slum01.tasset";
	TEngine::Get()->GetScene()->LoadSceneAync(TargetSceneAsset);
}
