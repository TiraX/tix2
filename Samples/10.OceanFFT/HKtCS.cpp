/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "HKtCS.h"
#include "OceanRenderer.h"

FHKtCS::FHKtCS()
	: FComputeTask("S_HKtCS")
{
}

FHKtCS::~FHKtCS()
{
}

static const E_PIXEL_FORMAT ResultFormat = EPF_RG32F;
void FHKtCS::PrepareResources(FRHI* RHI)
{
	InfoUniform = ti_new FHKtUniform;

	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	TTextureDesc HKtTextureDesc;
	HKtTextureDesc.Type = ETT_TEXTURE_2D;
	HKtTextureDesc.Format = ResultFormat;
	HKtTextureDesc.Width = FOceanRenderer::FFT_Size;
	HKtTextureDesc.Height = FOceanRenderer::FFT_Size;
	HKtTextureDesc.AddressMode = ETC_CLAMP_TO_EDGE;

	for (int i = 0; i < 3; ++i)
	{
		HKtResults[i] = RHI->CreateTexture(HKtTextureDesc);
		HKtResults[i]->SetTextureFlag(ETF_UAV, true);
		HKtResults[i]->SetResourceName("HKtResults");
		RHI->UpdateHardwareResourceTexture(HKtResults[i]);

		ResourceTable->PutRWTextureInTable(HKtResults[i], 0, UAV_HKT_X + i);
	}
}

void FHKtCS::UpdataComputeParams(
	FRHI* RHI,
	FTexturePtr InH0Texture,
	float InDepth,
	float InChoppyScale
)
{
	FHKtUniform::FUniformBufferStruct& UniformData = InfoUniform->UniformBufferData[0];
	if (UniformData.Info.X == 0.f ||
		UniformData.Info.Z != InDepth ||
		UniformData.Info.W != InChoppyScale
		)
	{
		UniformData.Info.X = FOceanRenderer::FFT_Size;
		UniformData.Info.Y = FOceanRenderer::FFT_Size;
		UniformData.Info.Z = InDepth;
		UniformData.Info.W = InChoppyScale;

		InfoUniform->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
	}


	if (H0Texture != InH0Texture)
	{
		ResourceTable->PutTextureInTable(InH0Texture, SRV_H0);
		H0Texture = InH0Texture;
	}
}

void FHKtCS::Run(FRHI* RHI)
{
	const uint32 BlockSize = 32;
	vector3di DispatchSize = vector3di(1, 1, 1);
	DispatchSize.X = (FOceanRenderer::FFT_Size + BlockSize - 1) / BlockSize;
	DispatchSize.Y = (FOceanRenderer::FFT_Size + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstant(0, FFloat4(FRenderThread::Get()->GetRenderThreadLiveTime() * 2.f, 0.f, 0.f, 0.f));
	RHI->SetComputeConstantBuffer(1, InfoUniform->UniformBuffer);
	RHI->SetComputeResourceTable(2, ResourceTable);

	RHI->DispatchCompute(vector3di(BlockSize, BlockSize, 1), DispatchSize);
}