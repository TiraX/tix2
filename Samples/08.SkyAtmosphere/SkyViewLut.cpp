/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SkyViewLut.h"

FSkyViewLutCS::FSkyViewLutCS()
	: FComputeTask("S_SkyViewLutCS")
{
}

FSkyViewLutCS::~FSkyViewLutCS()
{
}

void FSkyViewLutCS::PrepareResources(FRHI* RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	TTextureDesc LutDesc;
	LutDesc.Type = ETT_TEXTURE_2D;
	LutDesc.Format = EPF_R11G11B10F;
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

void FSkyViewLutCS::UpdataComputeParams(
	FRHI* RHI,
	FUniformBufferPtr InAtmosphereParam,
	FTexturePtr InTransmittanceLut,
	FTexturePtr InMultiScatteredLuminanceLut
)
{
	if (AtmosphereParam != InAtmosphereParam)
	{
		AtmosphereParam = InAtmosphereParam;
	}
	if (TransmittanceLut != InTransmittanceLut)
	{
		ResourceTable->PutTextureInTable(InTransmittanceLut, SRV_LUT_TRANSMITTANCE);
		TransmittanceLut = InTransmittanceLut;
	}
	if (MultiScatteredLuminanceLut != InMultiScatteredLuminanceLut)
	{
		ResourceTable->PutTextureInTable(InMultiScatteredLuminanceLut, SRV_LUT_MULTI_SCATTERED_LUMINANCE);
		MultiScatteredLuminanceLut = InMultiScatteredLuminanceLut;
	}
}

void FSkyViewLutCS::Run(FRHI* RHI)
{
	const uint32 BlockSize = 8;

	vector3di DispatchSize = vector3di(1, 1, 1);
	DispatchSize.X = (LUT_W + BlockSize - 1) / BlockSize;
	DispatchSize.Y = (LUT_H + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, AtmosphereParam);
	RHI->SetComputeResourceTable(1, ResourceTable);

	RHI->DispatchCompute(vector3di(BlockSize, BlockSize, 1), DispatchSize);
}