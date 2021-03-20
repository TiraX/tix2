/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "OceanRenderer.h"

static const int32 USE_HBAO = 0;
static const int32 USE_GTAO = 1;

static const int32 AO_METHOD = USE_GTAO;

FOceanRenderer::FOceanRenderer()
{
}

FOceanRenderer::~FOceanRenderer()
{
}

void FOceanRenderer::InitRenderFrame(FScene* Scene)
{
	// Prepare frame view uniform buffer
	Scene->InitRenderFrame();
}

void FOceanRenderer::InitInRenderThread()
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

	//GaussRandomCS = ti_new FGaussRandomCS();
	//GaussRandomCS->Finalize();
	//GaussRandomCS->PrepareResources(RHI);

	HZeroCS = ti_new FHZeroCS();
	HZeroCS->Finalize();
	HZeroCS->PrepareResources(RHI);

	AB_Result = RHI->CreateArgumentBuffer(1);
	{
		AB_Result->SetTexture(0, RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture);
		RHI->UpdateHardwareResourceAB(AB_Result, FSRender.GetFullScreenShader(), 0);
	}

	// Create resources
	CreateGaussRandomTexture();
}

void FOceanRenderer::CreateGaussRandomTexture()
{
	TI_ASSERT(GaussRandomTexture == nullptr);
	TImagePtr GaussRandomImage = ti_new TImage(EPF_RGBA16F, FFT_Size, FFT_Size);

	// Fill data
	TMath::RandSeed(FFT_Size  / 4);
	for (int32 y = 0; y < FFT_Size; ++y)
	{
		for (int32 x = 0; x < FFT_Size; ++x)
		{
			SColorf Result;

			float u0 = 2.f * PI * TMath::RandomUnit();
			float v0 = TMath::Sqrt(-2.f * log(TMath::RandomUnit()));
			float u1 = 2.f * PI * TMath::RandomUnit();
			float v1 = TMath::Sqrt(-2.f * log(TMath::RandomUnit()));

			Result.R = v0 * cos(u0);
			Result.G = v0 * sin(u0);
			Result.B = v1 * cos(u1);
			Result.A = v1 * sin(u1);

			GaussRandomImage->SetPixel(x, y, Result);
		}
	}

	// Create Texture
	TTextureDesc Desc;
	Desc.Format = EPF_RGBA16F;
	Desc.Width = FFT_Size;
	Desc.Height = FFT_Size;
	GaussRandomTexture = FRHI::Get()->CreateTexture(Desc);
	FRHI::Get()->UpdateHardwareResourceTexture(GaussRandomTexture, GaussRandomImage);

	// Debug
	//GaussRandomImage->SaveToHDR("GaussRandom.hdr");
;}

void FOceanRenderer::Render(FRHI* RHI, FScene* Scene)
{
	const float w = TMath::DegToRad(45);
	HZeroCS->UpdataComputeParams(
		RHI,
		GaussRandomTexture,
		40.f,
		30.f,
		vector2df(cos(w), sin(-w)),
		0.001f,
		0.5f
		);

	RHI->BeginComputeTask();
	{
		RHI->BeginEvent("HZero");
		HZeroCS->Run(RHI);
		RHI->EndEvent();
	}
	RHI->EndComputeTask();

	RHI->BeginRenderToRenderTarget(RT_BasePass, "BasePass");
	DrawSceneTiles(RHI, Scene);


	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}
