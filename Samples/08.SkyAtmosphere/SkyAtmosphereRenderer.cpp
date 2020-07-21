/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SkyAtmosphereRenderer.h"

FSkyAtmosphereRenderer::FSkyAtmosphereRenderer()
{
}

FSkyAtmosphereRenderer::~FSkyAtmosphereRenderer()
{
}

void FSkyAtmosphereRenderer::InitRenderFrame(FScene* Scene)
{
	// Prepare frame view uniform buffer
	Scene->InitRenderFrame();
}

void FSkyAtmosphereRenderer::InitInRenderThread()
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

	AB_Result = RHI->CreateArgumentBuffer(1);
	{
		AB_Result->SetTexture(0, RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture);
		RHI->UpdateHardwareResourceAB(AB_Result, FSRender.GetFullScreenShader(), 0);
	}

	// Init Lut Shaders
	TransmittanceCS = ti_new FTransmittanceLutCS();
	TransmittanceCS->Finalize();
	TransmittanceCS->PrepareResources(RHI);
}

void FSkyAtmosphereRenderer::DrawSceneTiles(FRHI* RHI, FScene * Scene)
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

void FSkyAtmosphereRenderer::Render(FRHI* RHI, FScene* Scene)
{
	RHI->BeginComputeTask();
	{
		RHI->BeginEvent("TransmittanceLut");
		TransmittanceCS->Run(RHI);
		RHI->EndEvent();
	}
	RHI->EndComputeTask();

	RHI->BeginRenderToRenderTarget(RT_BasePass, 0, "BasePass");
	DrawSceneTiles(RHI, Scene);

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}
