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
	LutDesc.Type = ETT_TEXTURE_3D;
	LutDesc.Format = EPF_RGBA16F;
	LutDesc.Width = LUT_W;
	LutDesc.Height = LUT_H;
	LutDesc.Depth = LUT_D;
	LutDesc.AddressMode = ETC_CLAMP_TO_EDGE;
	LutDesc.SRGB = 0;
	LutDesc.Mips = 1;

	CameraAerialPerspectiveVolumeLut = RHI->CreateTexture(LutDesc);
	CameraAerialPerspectiveVolumeLut->SetTextureFlag(ETF_UAV, true);
	CameraAerialPerspectiveVolumeLut->SetResourceName("CameraAerialPerspectiveVolumeLut");
	RHI->UpdateHardwareResourceTexture(CameraAerialPerspectiveVolumeLut);

	ResourceTable->PutRWTextureInTable(CameraAerialPerspectiveVolumeLut, 0, UAV_LUT_RESULT);
}

void FCameraVolumeLutCS::UpdataComputeParams(
	FRHI * RHI,
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

void FCameraVolumeLutCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 4;

	vector3di DispatchSize = vector3di(1, 1, 1);
	DispatchSize.X = (LUT_W + BlockSize - 1) / BlockSize;
	DispatchSize.Y = (LUT_H + BlockSize - 1) / BlockSize;
	DispatchSize.Z = (LUT_D + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, AtmosphereParam);
	RHI->SetComputeResourceTable(1, ResourceTable);

	RHI->DispatchCompute(vector3di(BlockSize, BlockSize, BlockSize), DispatchSize);
}