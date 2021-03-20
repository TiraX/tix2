/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "HZeroCS.h"
#include "OceanRenderer.h"

static const int RandTexSize = 64;

FHZeroCS::FHZeroCS()
	: FComputeTask("S_HZeroCS")
{
}

FHZeroCS::~FHZeroCS()
{
}

void FHZeroCS::PrepareResources(FRHI* RHI)
{
	InfoUniform = ti_new FHZeroUniform;

	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	TTextureDesc H0TextureDesc;
	H0TextureDesc.Type = ETT_TEXTURE_2D;
	H0TextureDesc.Format = EPF_RGBA16F;
	H0TextureDesc.Width = FOceanRenderer::FFT_Size;
	H0TextureDesc.Height = FOceanRenderer::FFT_Size;
	H0TextureDesc.AddressMode = ETC_CLAMP_TO_EDGE;

	H0Texture = RHI->CreateTexture(H0TextureDesc);
	H0Texture->SetTextureFlag(ETF_UAV, true);
	H0Texture->SetResourceName("H0Texture");
	RHI->UpdateHardwareResourceTexture(H0Texture);

	ResourceTable->PutRWTextureInTable(H0Texture, 0, UAV_H0_RESULT);
}

void FHZeroCS::UpdataComputeParams(
	FRHI* RHI,
	FTexturePtr InGaussTexture,
	float InWaveHeight,
	float InWindSpeed,
	const vector2df& InWindDir,
	float InSpressWaveLength,
	float InDamp
)
{
	float _L = InWindSpeed * InWindSpeed / 9.8f;

	FHZeroUniform::FUniformBufferStruct& UniformData = InfoUniform->UniformBufferData[0];
	if (UniformData.Info.X == 0.f ||
		UniformData.Info.Z != _L ||
		UniformData.Info.W != InWaveHeight ||
		UniformData.Info1.X != InWindDir.X ||
		UniformData.Info1.Y != InWindDir.Y ||
		UniformData.Info1.Z != InSpressWaveLength ||
		UniformData.Info1.W != InDamp
		)
	{
		UniformData.Info.X = FOceanRenderer::FFT_Size;
		UniformData.Info.Y = FOceanRenderer::FFT_Size;
		UniformData.Info.Z = _L;
		UniformData.Info.W = InWaveHeight;
		UniformData.Info1.X = InWindDir.X;
		UniformData.Info1.Y = InWindDir.Y;
		UniformData.Info1.Z = InSpressWaveLength;
		UniformData.Info1.W = InDamp;

		InfoUniform->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
	}


	if (GaussTexture != InGaussTexture)
	{
		ResourceTable->PutTextureInTable(InGaussTexture, SRV_GAUSS_RANDOM);
		GaussTexture = InGaussTexture;
	}
}

void FHZeroCS::Run(FRHI* RHI)
{
	const uint32 BlockSize = 32;
	vector3di DispatchSize = vector3di(1, 1, 1);
	DispatchSize.X = (FOceanRenderer::FFT_Size + BlockSize - 1) / BlockSize;
	DispatchSize.Y = (FOceanRenderer::FFT_Size + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, InfoUniform->UniformBuffer);
	RHI->SetComputeResourceTable(1, ResourceTable);

	RHI->DispatchCompute(vector3di(BlockSize, BlockSize, 1), DispatchSize);
}