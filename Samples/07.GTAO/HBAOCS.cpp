/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "HBAOCS.h"

FHBAOCS::FHBAOCS()
	: FComputeTask("S_HBAOCS")
{
}

FHBAOCS::~FHBAOCS()
{
}

void FHBAOCS::PrepareResources(FRHI * RHI)
{
	int32 RTW = RHI->GetViewport().Width;
	int32 RTH = RHI->GetViewport().Height;

	InfoUniform = ti_new FHBAOUniform;
	InfoUniform->UniformBufferData[0].ScreenSize = FFloat4(float(RTW), float(RTH), 1.f / RTW, 1.f / RTH);

	FScene * Scene = FRenderThread::Get()->GetRenderScene();
	const FViewProjectionInfo& VPInfo = Scene->GetViewProjection();
	
	float FocalLenX = 1.f / tanf(VPInfo.Fov * 0.5f) *  (float)RTH / (float)RTW;
	float FocalLenY = 1.f / tanf(VPInfo.Fov * 0.5f);
	InfoUniform->UniformBufferData[0].InvFocalLen = FFloat4(1.f / FocalLenX, 1.f / FocalLenY, 0, 0);

	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	TTextureDesc AOTextureDesc;
	AOTextureDesc.Type = ETT_TEXTURE_2D;
	AOTextureDesc.Format = EPF_R32F;
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
}

void FHBAOCS::UpdataComputeParams(
	FRHI * RHI,
	const vector3df& ViewDir,
	FTexturePtr InSceneTexture,
	FTexturePtr InSceneDepth
)
{
	InfoUniform->UniformBufferData[0].ViewDir = FFloat4(ViewDir.X, ViewDir.Y, ViewDir.Z, 1.f);
	InfoUniform->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

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

void FHBAOCS::Run(FRHI * RHI)
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