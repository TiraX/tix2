/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GTAOCS.h"

static const int RandTexSize = 64;

FGTAOCS::FGTAOCS()
	: FComputeTask("S_GTAOCS")
	, FoV(0.f)
{
}

FGTAOCS::~FGTAOCS()
{
}

void FGTAOCS::PrepareResources(FRHI * RHI)
{
	int32 RTW = RHI->GetViewport().Width;
	int32 RTH = RHI->GetViewport().Height;

	InfoUniform = ti_new FGTAOUniform;
	InfoUniform->UniformBufferData[0].ScreenSize = FFloat4(float(RTW), float(RTH), 1.f / RTW, 1.f / RTH);
	const float R = 0.1f;
	InfoUniform->UniformBufferData[0].Radius = FFloat4(R, R * R, 1.f / R, 1.f);
	
	//float Inc = 2.0f * PI / (float)MAX_DIR;
	//for (int i = 0; i < MAX_DIR; i ++)
	//{
	//	float Angle = Inc * i;
	//	InfoUniform->UniformBufferData[0].Dirs[i] = FFloat2(cos(Angle), sin(Angle));
	//}

	FScene * Scene = FRenderThread::Get()->GetRenderScene();

	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	TTextureDesc AOTextureDesc;
	AOTextureDesc.Type = ETT_TEXTURE_2D;
	AOTextureDesc.Format = EPF_RGBA32F;
	AOTextureDesc.Width = RHI->GetViewport().Width;
	AOTextureDesc.Height = RHI->GetViewport().Height;
	AOTextureDesc.AddressMode = ETC_CLAMP_TO_EDGE;
	AOTextureDesc.SRGB = 0;
	AOTextureDesc.Mips = 1;

	AOTexture = RHI->CreateTexture(AOTextureDesc);
	AOTexture->SetTextureFlag(ETF_UAV, true);
	AOTexture->SetResourceName("AOTexture");
	RHI->UpdateHardwareResourceTexture(AOTexture);

	ResourceTable->PutRWTextureInTable(AOTexture, 0, UAV_AO_RESULT);

	// Create random texture
	TTextureDesc Desc;
	Desc.Format = EPF_RGBA8;
	Desc.Width = RandTexSize;
	Desc.Height = RandTexSize;
	RandomTex = RHI->CreateTexture(Desc);

	TImagePtr RandomImage = ti_new TImage(EPF_RGBA8, RandTexSize, RandTexSize);
	for (int32 y = 0; y < RandTexSize; ++y)
	{
		for (int32 x = 0; x < RandTexSize; ++x)
		{
			float Angle = 2.f * PI * TMath::RandomUnit() / (float)MAX_DIR;
			SColor R;
			R.R = (uint8)(cos(Angle) * 127.5f + 127.5f);
			R.G = (uint8)(sin(Angle) * 127.5f + 127.5f);
			R.B = (uint8)(TMath::RandomUnit() * 127.5f + 127.5f);
			//R.B = (uint8)(randomUnit() * 255.f);
			RandomImage->SetPixel(x, y, R);
		}
	}
	RHI->UpdateHardwareResourceTexture(RandomTex, RandomImage);
	ResourceTable->PutTextureInTable(RandomTex, SRV_RANDOM_TEXTURE);
}

void FGTAOCS::UpdataComputeParams(
	FRHI * RHI,
	float InFov,
	FTexturePtr InSceneTexture,
	FTexturePtr InSceneDepth
)
{
	if (InFov != FoV)
	{
		FoV = InFov;
		int32 RTW = RHI->GetViewport().Width;
		int32 RTH = RHI->GetViewport().Height;

		float FocalLenX = 1.f / tanf(InFov * 0.5f) * (float)RTH / (float)RTW;
		float FocalLenY = 1.f / tanf(InFov * 0.5f);
		InfoUniform->UniformBufferData[0].FocalLen = FFloat4(FocalLenX, FocalLenY, 1.f / FocalLenX, 1.f / FocalLenY);

		InfoUniform->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
	}

	if (SceneNormal != InSceneTexture)
	{
		ResourceTable->PutTextureInTable(InSceneTexture, SRV_SCENE_NORMAL);
		SceneNormal = InSceneTexture;
	}
	if (SceneDepth != InSceneDepth)
	{
		ResourceTable->PutTextureInTable(InSceneDepth, SRV_SCENE_DEPTH);
		SceneDepth = InSceneDepth;
	}
}

void FGTAOCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 8;
	int32 RTW = RHI->GetViewport().Width;
	int32 RTH = RHI->GetViewport().Height;

	vector3di DispatchSize = vector3di(1, 1, 1);
	DispatchSize.X = (RTW + BlockSize - 1) / BlockSize;
	DispatchSize.Y = (RTH + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, InfoUniform->UniformBuffer);
	RHI->SetComputeResourceTable(1, ResourceTable);

	RHI->DispatchCompute(vector3di(BlockSize, BlockSize, 1), DispatchSize);
}