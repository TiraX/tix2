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

	MeanIllumLutCS = ti_new FMeanIllumLutCS();
	MeanIllumLutCS->Finalize();
	MeanIllumLutCS->PrepareResources(RHI);

	// Init Atmosphere uniform param
	AtmosphereParam = ti_new FAtmosphereParam;
	AtmosphereParam->UniformBufferData[0].TransmittanceLutSizeAndInv = 
		FFloat4(float(FTransmittanceLutCS::LUT_W), float(FTransmittanceLutCS::LUT_H), 
		1.f / float(FTransmittanceLutCS::LUT_W), 1.f / float(FTransmittanceLutCS::LUT_H));
	AtmosphereParam->UniformBufferData[0].MultiScatteredLuminanceLutSizeAndInv = 
		FFloat4(float(FMeanIllumLutCS::LUT_W), float(FMeanIllumLutCS::LUT_H), 
		1.f / float(FMeanIllumLutCS::LUT_W), 1.f / float(FMeanIllumLutCS::LUT_H));
	const float TopRadiusKm = 6420.f;
	const float BottomRadiusKm = 6360.f;
	AtmosphereParam->UniformBufferData[0].RadiusRange = FFloat4(TopRadiusKm, BottomRadiusKm, sqrt(TopRadiusKm * TopRadiusKm - BottomRadiusKm * BottomRadiusKm), BottomRadiusKm * BottomRadiusKm);
	AtmosphereParam->UniformBufferData[0].MieRayleigh = FFloat4(0.8f, -0.83333f, -0.125f, 1.f);
	AtmosphereParam->UniformBufferData[0].Params1 = FFloat4(10.f, 1.f, 25.f, 15.f);
	AtmosphereParam->UniformBufferData[0].AbsorptionDensity01MA = FFloat4(0.06667f, -0.6667f, -0.06667f, 2.66667f);
	AtmosphereParam->UniformBufferData[0].MieScattering = FFloat4(0.004f, 0.004f, 0.004f, 0.004f);
	AtmosphereParam->UniformBufferData[0].MieAbsorption = FFloat4(0.00044f, 0.00044f, 0.00044f, 0.00044f);
	AtmosphereParam->UniformBufferData[0].MieExtinction = FFloat4(0.00444f, 0.00444f, 0.00444f, 0.00444f);
	AtmosphereParam->UniformBufferData[0].RayleighScattering = FFloat4(0.0058f, 0.01356f, 0.0331f, 1.f);
	AtmosphereParam->UniformBufferData[0].AbsorptionExtinction = FFloat4(0.00065f, 0.00188f, 0.00008f, 1.f);
	AtmosphereParam->UniformBufferData[0].GroundAlbedo = FFloat4(0.40198f, 0.40198f, 0.40198f, 1.f);
	AtmosphereParam->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
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
	TransmittanceCS->UpdataComputeParams(RHI, AtmosphereParam->UniformBuffer);
	MeanIllumLutCS->UpdataComputeParams(RHI, AtmosphereParam->UniformBuffer, TransmittanceCS->GetTransmittanceLutTexture());

	RHI->BeginComputeTask();
	{
		RHI->BeginEvent("TransmittanceLut");
		TransmittanceCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("MeanIllumLut");
		MeanIllumLutCS->Run(RHI);
		RHI->EndEvent();
	}
	RHI->EndComputeTask();

	RHI->BeginRenderToRenderTarget(RT_BasePass, 0, "BasePass");
	DrawSceneTiles(RHI, Scene);

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}
