/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GTAORenderer.h"

static const int32 USE_HBAO = 0;
static const int32 USE_GTAO = 1;

static const int32 AO_METHOD = USE_GTAO;

FGTAORenderer::FGTAORenderer()
{
}

FGTAORenderer::~FGTAORenderer()
{
}

void FGTAORenderer::InitRenderFrame(FScene* Scene)
{
	// Prepare frame view uniform buffer
	Scene->InitRenderFrame();
}

void FGTAORenderer::InitInRenderThread()
{
	FDefaultRenderer::InitInRenderThread();

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

	// Load default pipeline
	const TString DefaultMaterial = "M_Debug.tasset";
	TAssetPtr DebugMaterialAsset = TAssetLibrary::Get()->LoadAsset(DefaultMaterial);
	TResourcePtr DebugMaterialResource = DebugMaterialAsset->GetResourcePtr();
	TMaterialPtr DebugMaterial = static_cast<TMaterial*>(DebugMaterialResource.get());
	FPipelinePtr DebugPipeline = DebugMaterial->PipelineResource;

	HBAOCompute = ti_new FHBAOCS();
	HBAOCompute->Finalize();
	HBAOCompute->PrepareResources(RHI);

	GTAOCompute = ti_new FGTAOCS();
	GTAOCompute->Finalize();
	GTAOCompute->PrepareResources(RHI);

	AB_Result = RHI->CreateArgumentBuffer(1);
	{
		//AB_Result->SetTexture(0, RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture);
		if (AO_METHOD == USE_GTAO)
			AB_Result->SetTexture(0, GTAOCompute->GetAOTexture());
		else
			AB_Result->SetTexture(0, HBAOCompute->GetAOTexture());
		RHI->UpdateHardwareResourceAB(AB_Result, FSRender.GetFullScreenShader(), 0);
	}
}

void FGTAORenderer::DrawSceneTiles(FRHI* RHI, FScene * Scene)
{
	const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();
	for (auto& TileIter : SceneTileResources)
	{
		const vector2di& TilePos = TileIter.first;
		FSceneTileResourcePtr TileRes = TileIter.second;

		const TVector<FPrimitivePtr>& TilePrimitives = TileRes->GetPrimitives();
		for (uint32 PIndex = 0; PIndex < (uint32)TilePrimitives.size(); ++PIndex)
		{
			FPrimitivePtr Primitive = TilePrimitives[PIndex];

			if (Primitive != nullptr)
			{
				FInstanceBufferPtr InstanceBuffer = Primitive->GetInstanceBuffer();
				RHI->SetGraphicsPipeline(Primitive->GetPipeline());
				RHI->SetMeshBuffer(Primitive->GetMeshBuffer(), InstanceBuffer);
				ApplyShaderParameter(RHI, Scene, Primitive);

				RHI->DrawPrimitiveIndexedInstanced(
					Primitive->GetMeshBuffer(),
					InstanceBuffer == nullptr ? 1 : Primitive->GetInstanceCount(),
					Primitive->GetInstanceOffset());
			}
		}
	}
}

void FGTAORenderer::Render(FRHI* RHI, FScene* Scene)
{
	RHI->BeginRenderToRenderTarget(RT_BasePass, "BasePass-Normal");
	DrawSceneTiles(RHI, Scene);

	if (AO_METHOD == USE_GTAO)
	{
		GTAOCompute->UpdataComputeParams(
			RHI,
			Scene->GetViewProjection().Fov,
			RT_BasePass->GetColorBuffer(0).Texture,
			RT_BasePass->GetDepthStencilBuffer().Texture);

		RHI->BeginComputeTask();
		{
			RHI->BeginEvent("GTAO");
			GTAOCompute->Run(RHI);
			RHI->EndEvent();
		}
		RHI->EndComputeTask();
	}
	else
	{
		HBAOCompute->UpdataComputeParams(
			RHI,
			Scene->GetViewProjection().Fov,
			RT_BasePass->GetColorBuffer(0).Texture,
			RT_BasePass->GetDepthStencilBuffer().Texture);

		RHI->BeginComputeTask();
		{
			RHI->BeginEvent("HBAO");
			HBAOCompute->Run(RHI);
			RHI->EndEvent();
		}
		RHI->EndComputeTask();
	}

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}
