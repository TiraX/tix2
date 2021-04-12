/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GenerateNormalCS.h"
#include "OceanRenderer.h"

FGenerateNormalCS::FGenerateNormalCS()
	: FComputeTask("S_GenerateNormalCS")
{
}

FGenerateNormalCS::~FGenerateNormalCS()
{
}

static const E_PIXEL_FORMAT ResultFormat = EPF_RGBA8;
void FGenerateNormalCS::PrepareResources(FRHI* RHI)
{
	InfoUniform = ti_new FGenerateNormalUniform();
	InfoUniform->UniformBufferData[0].Info.X = FOceanRenderer::FFT_Size;
	InfoUniform->UniformBufferData[0].Info.Y = FOceanRenderer::FFT_Size;
	InfoUniform->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	TTextureDesc NormalTextureDesc;
	NormalTextureDesc.Type = ETT_TEXTURE_2D;
	NormalTextureDesc.Format = ResultFormat;
	NormalTextureDesc.Width = FOceanRenderer::FFT_Size;
	NormalTextureDesc.Height = FOceanRenderer::FFT_Size;
	NormalTextureDesc.AddressMode = ETC_REPEAT;

	NormalTexture = RHI->CreateTexture(NormalTextureDesc);
	NormalTexture->SetTextureFlag(ETF_UAV, true);
	NormalTexture->SetResourceName("NormalTexture");
	RHI->UpdateHardwareResourceTexture(NormalTexture);

	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	ResourceTable->PutRWTextureInTable(NormalTexture, 0, UAV_NORMAL_TEXTURE);
}

void FGenerateNormalCS::UpdataComputeParams(
	FRHI* RHI,
	FTexturePtr InDisplacement
)
{
	if (InDisplacement != DisplacementTexture)
	{
		ResourceTable->PutTextureInTable(InDisplacement, SRV_DISPLACEMENT);
		DisplacementTexture = InDisplacement;
	}
}

void FGenerateNormalCS::Run(FRHI* RHI)
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