/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "DistantSkyLightLut.h"

FDistantSkyLightLutCS::FDistantSkyLightLutCS()
	: FComputeTask("S_DistantSkyLightLutCS")
{
}

FDistantSkyLightLutCS::~FDistantSkyLightLutCS()
{
}

void FDistantSkyLightLutCS::PrepareResources(FRHI* RHI)
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

	DistantSkyLightLut = RHI->CreateTexture(LutDesc);
	DistantSkyLightLut->SetTextureFlag(ETF_UAV, true);
	DistantSkyLightLut->SetResourceName("DistantSkyLightLut");
	RHI->UpdateHardwareResourceTexture(DistantSkyLightLut);

	ResourceTable->PutRWTextureInTable(DistantSkyLightLut, 0, UAV_LUT_RESULT);

	const uint32 GroupSize = 8;
	const float GroupSizeInv = 1.f / GroupSize;
	srand(0xDE4DC0DE);
	FFloat4* Data = ti_new FFloat4[GroupSize * GroupSize];
	for (uint32 i = 0; i < GroupSize; ++i)
	{
		for (uint32 j = 0; j < GroupSize; ++j)
		{
			const float u0 = (float(i) + randomUnit()) * GroupSizeInv;
			const float u1 = (float(j) + randomUnit()) * GroupSizeInv;

			const float a = 1.0f - 2.0f * u0;
			const float b = sqrt(1.0f - a * a);
			const float phi = 2 * PI * u1;

			uint32 idx = j * GroupSize + i;
			Data[idx].X = b * cos(phi);
			Data[idx].Y = b * sin(phi);
			Data[idx].Z = a;
			Data[idx].W = 0.0f;
		}
	}
	UniformSphereSamplesBuffer = RHI->CreateUniformBuffer(sizeof(FFloat4), GroupSize * GroupSize, 0);
	UniformSphereSamplesBuffer->SetResourceName("UniformSphereSamplesBuffer");
	RHI->UpdateHardwareResourceUB(UniformSphereSamplesBuffer, Data);
	ResourceTable->PutUniformBufferInTable(UniformSphereSamplesBuffer, SRV_SPHERE_SAMPLES);
	ti_delete[] Data;
}

void FDistantSkyLightLutCS::UpdataComputeParams(
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

void FDistantSkyLightLutCS::Run(FRHI* RHI)
{
	const uint32 BlockSize = 8;

	vector3di DispatchSize = vector3di(1, 1, 1);

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, AtmosphereParam);
	RHI->SetComputeResourceTable(1, ResourceTable);

	RHI->DispatchCompute(vector3di(BlockSize, BlockSize, 1), DispatchSize);
}