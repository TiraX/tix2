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
	CreateButterFlyTexture();
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
	Desc.Format = GaussRandomImage->GetFormat();
	Desc.Width = FFT_Size;
	Desc.Height = FFT_Size;
	GaussRandomTexture = FRHI::Get()->CreateTexture(Desc);
	FRHI::Get()->UpdateHardwareResourceTexture(GaussRandomTexture, GaussRandomImage);

	// Debug
	//GaussRandomImage->SaveToHDR("GaussRandom.hdr");
;}

void FOceanRenderer::CreateButterFlyTexture()
{
	TI_ASSERT(ButterFlyTexture == nullptr);
	const int H = FFT_Size;
	const int W = TMath::FloorLog2(H);
	TImagePtr ButterFlyImage = ti_new TImage(EPF_RGBA32F, W, H);

	TVector<int32> BitReversed;
	BitReversed.resize(H);
	memset(BitReversed.data(), 0, H * sizeof(int32));
	for (int32 i = 0; i < W; ++i)
	{
		int32 Stride = 1 << i;
		int32 Add = 1 << (W - 1 - i);
		for (int32 j = 0; j < H; ++j)
		{
			if ((j / Stride) % 2 != 0)
				BitReversed[j] += Add;
		}
	}

	for (int32 y = 0; y < H; ++y)
	{
		for (int32 x = 0; x < W; ++x)
		{
			float k = (float)(y * H / (int32)pow(2, (x + 1)) % H);
			//float k = (float)(TMath::FMod(y * H / pow(2.f, x + 1), H));

			vector2df Twiddle;
			float val = 2.f * PI * k / H;
			Twiddle.X = cos(val);
			Twiddle.Y = sin(val);

			int32 ButterFlySpan = (int32)pow(2, x);

			int32 ButterFlyWing = 0;
			if (y % ((int32)pow(2, (x + 1))) < (int32)pow(2, x))
				ButterFlyWing = 1;

			if (x == 0)
			{
				if (ButterFlyWing == 1)
				{
					int32 Next = (y + 1) % H;
					ButterFlyImage->SetPixel(x, y, SColorf(Twiddle.X, Twiddle.Y, (float)BitReversed[y], (float)BitReversed[Next]));
				}
				else
				{
					int32 Prev = (y - 1) < 0 ? y : y - 1;
					ButterFlyImage->SetPixel(x, y, SColorf(Twiddle.X, Twiddle.Y, (float)BitReversed[Prev], (float)BitReversed[y]));
				}
			}
			else
			{
				if (ButterFlyWing == 1)
				{
					int32 Next = (y + ButterFlySpan) % H;
					ButterFlyImage->SetPixel(x, y, SColorf(Twiddle.X, Twiddle.Y, float(y), (float)(y + ButterFlySpan)));
				}
				else
				{
					int32 Prev = (y - ButterFlySpan) < 0 ? (y - ButterFlySpan + H) : (y - ButterFlySpan);
					ButterFlyImage->SetPixel(x, y, SColorf(Twiddle.X, Twiddle.Y, float(Prev), float(y)));
				}
			}
		}
	}
	// Create Texture
	TTextureDesc Desc;
	Desc.Format = ButterFlyImage->GetFormat();
	Desc.Width = W;
	Desc.Height = H;
	ButterFlyTexture = FRHI::Get()->CreateTexture(Desc);
	FRHI::Get()->UpdateHardwareResourceTexture(ButterFlyTexture, ButterFlyImage);

	//// Debug
	//SColorf* Data = (SColorf*)ButterFlyImage->Lock();
	//for (int y = 0; y < H; ++y)
	//{
	//	for (int x = 0; x < W; ++x)
	//	{
	//		_LOG(Log, "%0.2f %0.2f %0.2f %0.2f; ", Data->R, Data->G, Data->B, Data->A);
	//		++Data;
	//	}
	//	_LOG(Log, "\n");
	//}
	//TImagePtr ButterFlyTGA = ti_new TImage(EPF_RGBA8, W, H);
	//for (int y = 0; y < H; ++y)
	//{
	//	for (int x = 0; x < W; ++x)
	//	{
	//		SColorf c = ButterFlyImage->GetPixelFloat(x, y);
	//		if (c.B > 1.f) c.B = 1.f;
	//		if (c.A > 1.f) c.A = 1.f;
	//		if (c.R < 0.f) c.R = 0.f;
	//		if (c.G < 0.f) c.G = 0.f;
	//		SColor cc;
	//		c.ToSColor(cc);
	//		ButterFlyTGA->SetPixel(x, y, cc);
	//	}
	//}

	//ButterFlyTGA->SaveToTGA("ButterFlyImage.tga");
}

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
