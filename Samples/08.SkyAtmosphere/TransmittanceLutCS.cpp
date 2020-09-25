/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TransmittanceLutCS.h"

FTransmittanceLutCS::FTransmittanceLutCS()
	: FComputeTask("S_TransmittanceLutCS")
{
}

FTransmittanceLutCS::~FTransmittanceLutCS()
{
}

void FTransmittanceLutCS::PrepareResources(FRHI* RHI)
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

	TransmittanceLut = RHI->CreateTexture(LutDesc);
	TransmittanceLut->SetTextureFlag(ETF_UAV, true);
	TransmittanceLut->SetResourceName("TransmittanceLut");
	RHI->UpdateHardwareResourceTexture(TransmittanceLut);

	ResourceTable->PutRWTextureInTable(TransmittanceLut, 0, UAV_LUT_RESULT);
}

void FTransmittanceLutCS::UpdataComputeParams(FRHI* RHI, FUniformBufferPtr InAtomsphereParam)
{
	if (AtmosphereParam != InAtomsphereParam)
	{
		AtmosphereParam = InAtomsphereParam;
	}
}

void FTransmittanceLutCS::Run(FRHI* RHI)
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