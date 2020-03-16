/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GPUDrivenRenderer.h"
#include "SceneMetaInfos.h"

FGPUDrivenRenderer * GPUDrivenRenderer = nullptr;
FGPUDrivenRenderer * FGPUDrivenRenderer::Get()
{
	return GPUDrivenRenderer;
}

FGPUDrivenRenderer::FGPUDrivenRenderer()
	: SceneMetaInfo(nullptr)
{
	GPUDrivenRenderer = this;
	SceneMetaInfo = ti_new FSceneMetaInfos();
}

FGPUDrivenRenderer::~FGPUDrivenRenderer()
{
	GPUDrivenRenderer = nullptr;
	ti_delete SceneMetaInfo;
}

void FGPUDrivenRenderer::InitRenderFrame(FScene* Scene)
{
	// Prepare frame view uniform buffer
	Scene->InitRenderFrame();
}

void FGPUDrivenRenderer::InitInRenderThread()
{
	FRHI * RHI = FRHI::Get();
	FSRender.InitCommonResources(RHI);

	const int32 RTWidth = 1600;
	const int32 RTHeight = TEngine::AppInfo.Height * RTWidth / TEngine::AppInfo.Width;

	TStreamPtr ArgumentValues = ti_new TStream;
	TVector<FTexturePtr> ArgumentTextures;

	// Setup base pass render target
	RT_BasePass = FRenderTarget::Create(RTWidth, RTHeight);
	RT_BasePass->SetResourceName("RT_BasePass");
	RT_BasePass->AddColorBuffer(EPF_RGBA16F, 1, ERTC_COLOR0, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_BasePass->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8, 1, ERT_LOAD_CLEAR, ERT_STORE_DONTCARE);
	RT_BasePass->Compile();

	AB_Result = RHI->CreateArgumentBuffer(1);
	{
		AB_Result->SetTexture(0, RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture);
		RHI->UpdateHardwareResourceAB(AB_Result, FSRender.GetFullScreenShader(), 0);
	}

	// Load default pipeline
	const TString DefaultMaterial = "M_Debug.tasset";
	TAssetPtr DebugMaterialAsset = TAssetLibrary::Get()->LoadAsset(DefaultMaterial);
	TResourcePtr DebugMaterialResource = DebugMaterialAsset->GetResourcePtr();
	TMaterialPtr DebugMaterial = static_cast<TMaterial*>(DebugMaterialResource.get());
	FPipelinePtr DebugPipeline = DebugMaterial->PipelineResource;

	// Load Pre-Z pipeline.
	//const TString DepthOnlyMaterialName = "M_DepthOnly.tasset";
	//TAssetPtr DepthOnlyMaterialAsset = TAssetLibrary::Get()->LoadAsset(DepthOnlyMaterialName);
	//TResourcePtr DepthOnlyMaterialResource = DepthOnlyMaterialAsset->GetResourcePtr();
	//TMaterialPtr DepthOnlyMaterial = static_cast<TMaterial*>(DepthOnlyMaterialResource.get());
	//DepthOnlyPipeline = DepthOnlyMaterial->PipelineResource;

	// Init GPU command buffer
	TVector<E_GPU_COMMAND_TYPE> CommandStructure;
	CommandStructure.resize(1);
	CommandStructure[0] = GPU_COMMAND_DRAW_INDEXED;

	GPUCommandSignature = RHI->CreateGPUCommandSignature(DebugPipeline, CommandStructure);
	RHI->UpdateHardwareResourceGPUCommandSig(GPUCommandSignature);
	
	// Prepare compute cull tasks
	FScene * Scene = FRenderThread::Get()->GetRenderScene();
}

void FGPUDrivenRenderer::TestDrawSceneIndirectCommandBuffer(FRHI * RHI, FScene * Scene)
{
	if (SceneMetaInfo->GetGPUCommandBuffer() != nullptr)
	{
		RHI->SetResourceStateCB(SceneMetaInfo->GetGPUCommandBuffer(), RESOURCE_STATE_INDIRECT_ARGUMENT);
		RHI->SetMeshBuffer(SceneMetaInfo->GetMergedSceneMeshBuffer(), SceneMetaInfo->GetMergedInstanceBuffer());
		RHI->SetGraphicsPipeline(GPUCommandSignature->GetPipeline());
		RHI->SetUniformBuffer(ESS_VERTEX_SHADER, 0, Scene->GetViewUniformBuffer()->UniformBuffer);
		RHI->ExecuteGPUDrawCommands(SceneMetaInfo->GetGPUCommandBuffer());
	}
}

void FGPUDrivenRenderer::Render(FRHI* RHI, FScene* Scene)
{
	// Merge all instances and meshes into a big one
	SceneMetaInfo->PrepareSceneResources(RHI, Scene, GPUCommandSignature);

	TI_TODO("Use depth from last frame. Think carefully about it. NOT a clear solution yet.");
	{
		// Do test
		RHI->BeginRenderToRenderTarget(RT_BasePass, 0, "BasePass");
		TestDrawSceneIndirectCommandBuffer(RHI, Scene);
	}
	// Frustum cull instances before preZ

	// Render preZ depth

	// Option1: Instance occlusion cull / Cluster cull / Triangle cull

	// Option2: Cluster cull / Triangle cull

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}
