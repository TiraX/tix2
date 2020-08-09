/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "CameraVolumeLut.h"

FCameraVolumeLutCS::FCameraVolumeLutCS()
	: FComputeTask("S_CameraVolumeLutCS")
{
}

FCameraVolumeLutCS::~FCameraVolumeLutCS()
{
}

void FCameraVolumeLutCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	TTextureDesc LutDesc;
	LutDesc.Type = ETT_TEXTURE_2D;
	TI_TODO("Use R11G11B10 Format for optimization");
	LutDesc.Format = EPF_RGBA16F;
	LutDesc.Width = LUT_W;
	LutDesc.Height = LUT_H;
	LutDesc.AddressMode = ETC_CLAMP_TO_EDGE;
	LutDesc.SRGB = 0;
	LutDesc.Mips = 1;

	SkyViewLut = RHI->CreateTexture(LutDesc);
	SkyViewLut->SetTextureFlag(ETF_UAV, true);
	SkyViewLut->SetResourceName("SkyViewLut");
	RHI->UpdateHardwareResourceTexture(SkyViewLut);

	ResourceTable->PutRWTextureInTable(SkyViewLut, 0, UAV_LUT_RESULT);
}

void FCameraVolumeLutCS::UpdataComputeParams(
	FRHI * RHI
)
{
}

void FCameraVolumeLutCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 8;
}