/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "MeanIllumLutCS.h"

FMeanIllumLutCS::FMeanIllumLutCS()
	: FComputeTask("S_MeanIllumLutCS")
{
}

FMeanIllumLutCS::~FMeanIllumLutCS()
{
}

void FMeanIllumLutCS::PrepareResources(FRHI* RHI)
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

	MultiScatteredLuminanceLut = RHI->CreateTexture(LutDesc);
	MultiScatteredLuminanceLut->SetTextureFlag(ETF_UAV, true);
	MultiScatteredLuminanceLut->SetResourceName("MultiScatteredLuminanceLut");
	RHI->UpdateHardwareResourceTexture(MultiScatteredLuminanceLut);

	ResourceTable->PutRWTextureInTable(MultiScatteredLuminanceLut, 0, UAV_LUT_RESULT);
}

void FMeanIllumLutCS::UpdataComputeParams(FRHI* RHI, 
	FUniformBufferPtr InAtomsphereParam,
	FTexturePtr InTransmittanceLut)
{
	if (AtmosphereParam != InAtomsphereParam)
	{
		AtmosphereParam = InAtomsphereParam;
	}
	if (TransmittanceLut != InTransmittanceLut)
	{
		ResourceTable->PutTextureInTable(InTransmittanceLut, SRV_LUT_TRANSMITTANCE);
		TransmittanceLut = InTransmittanceLut;
	}
}

void FMeanIllumLutCS::Run(FRHI* RHI)
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