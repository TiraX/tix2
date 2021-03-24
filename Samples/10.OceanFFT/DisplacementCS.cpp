/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "DisplacementCS.h"
#include "OceanRenderer.h"

FDisplacementCS::FDisplacementCS()
	: FComputeTask("S_DisplacementCS")
{
}

FDisplacementCS::~FDisplacementCS()
{
}

static const E_PIXEL_FORMAT ResultFormat = EPF_RGBA32F;
void FDisplacementCS::PrepareResources(FRHI* RHI)
{
	InfoUniform = ti_new FDisplacementUniform();
	InfoUniform->UniformBufferData[0].Info.X = FOceanRenderer::FFT_Size;
	InfoUniform->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	TTextureDesc DisplacementTextureDesc;
	DisplacementTextureDesc.Type = ETT_TEXTURE_2D;
	DisplacementTextureDesc.Format = ResultFormat;
	DisplacementTextureDesc.Width = FOceanRenderer::FFT_Size;
	DisplacementTextureDesc.Height = FOceanRenderer::FFT_Size;
	DisplacementTextureDesc.AddressMode = ETC_CLAMP_TO_EDGE;

	DisplacementTexture = RHI->CreateTexture(DisplacementTextureDesc);
	DisplacementTexture->SetTextureFlag(ETF_UAV, true);
	DisplacementTexture->SetResourceName("DisplacementTexture");
	RHI->UpdateHardwareResourceTexture(DisplacementTexture);

	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	ResourceTable->PutRWTextureInTable(DisplacementTexture, 0, UAV_DISPLACEMENT_TEXTURE);
}

void FDisplacementCS::UpdataComputeParams(
	FRHI* RHI,
	FTexturePtr InIFFT_X,
	FTexturePtr InIFFT_Y,
	FTexturePtr InIFFT_Z
)
{
	if (InIFFT_X != IFFTTextures[0] ||
		InIFFT_Y != IFFTTextures[1] ||
		InIFFT_Z != IFFTTextures[2])
	{
		ResourceTable->PutTextureInTable(InIFFT_Z, SRV_IFFT_X);
		IFFTTextures[0] = InIFFT_X;
		IFFTTextures[1] = InIFFT_Y;
		IFFTTextures[2] = InIFFT_Z;
	}
}

void FDisplacementCS::Run(FRHI* RHI)
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