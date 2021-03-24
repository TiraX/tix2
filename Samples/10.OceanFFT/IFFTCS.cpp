/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "IFFTCS.h"
#include "OceanRenderer.h"

FIFFTCS::FIFFTCS()
	: FComputeTask("S_IFFTCS")
{
}

FIFFTCS::~FIFFTCS()
{
}

static const E_PIXEL_FORMAT ResultFormat = EPF_RG32F;
void FIFFTCS::PrepareResources(FRHI* RHI)
{
	const int32 Stages = TMath::FloorLog2(FOceanRenderer::FFT_Size);

	// Create uniform buffer for each stages
	InfoUniformHorizontal.resize(Stages);
	InfoUniformVertical.resize(Stages);
	for (int32 i = 0; i < Stages; i++)
	{
		InfoUniformHorizontal[i] = ti_new FIFFTUniform();
		InfoUniformHorizontal[i]->UniformBufferData[0].Info.X = 0;
		InfoUniformHorizontal[i]->UniformBufferData[0].Info.Y = (float)i;
		InfoUniformHorizontal[i]->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

		InfoUniformVertical[i] = ti_new FIFFTUniform();
		InfoUniformVertical[i]->UniformBufferData[0].Info.X = 1;
		InfoUniformVertical[i]->UniformBufferData[0].Info.Y = (float)i;
		InfoUniformVertical[i]->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
	}

	TTextureDesc OutputTextureDesc;
	OutputTextureDesc.Type = ETT_TEXTURE_2D;
	OutputTextureDesc.Format = ResultFormat;
	OutputTextureDesc.Width = FOceanRenderer::FFT_Size;
	OutputTextureDesc.Height = FOceanRenderer::FFT_Size;
	OutputTextureDesc.AddressMode = ETC_CLAMP_TO_EDGE;

	PingPongTempTexture = RHI->CreateTexture(OutputTextureDesc);
	PingPongTempTexture->SetTextureFlag(ETF_UAV, true);
	PingPongTempTexture->SetResourceName("IFFT-Pingpong");
	RHI->UpdateHardwareResourceTexture(PingPongTempTexture);

	ResourceTables[PING] = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
	ResourceTables[PONG] = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	ResourceTables[PING]->PutRWTextureInTable(PingPongTempTexture, 0, UAV_RESULT_TEXTURE);
	ResourceTables[PONG]->PutTextureInTable(PingPongTempTexture, SRV_SORUCE_TEXTURE);
}

void FIFFTCS::UpdataComputeParams(
	FRHI* RHI,
	FTexturePtr InHKtTexture,
	FTexturePtr InButterFlyTexture
)
{
	if (HKtTexture != InHKtTexture)
	{
		ResourceTables[PING]->PutTextureInTable(InHKtTexture, SRV_SORUCE_TEXTURE);
		ResourceTables[PONG]->PutRWTextureInTable(InHKtTexture, 0, UAV_RESULT_TEXTURE);
		HKtTexture = InHKtTexture;
	}
	if (ButterFlyTexture != InButterFlyTexture)
	{
		ResourceTables[PING]->PutTextureInTable(InButterFlyTexture, SRV_BUTTER_FLY);
		ResourceTables[PONG]->PutTextureInTable(InButterFlyTexture, SRV_BUTTER_FLY);
		ButterFlyTexture = InButterFlyTexture;
	}
}

void FIFFTCS::Run(FRHI* RHI)
{
	const int32 Stages = TMath::FloorLog2(FOceanRenderer::FFT_Size);
	const uint32 BlockSize = 32;
	vector3di DispatchSize = vector3di(1, 1, 1);
	DispatchSize.X = (FOceanRenderer::FFT_Size + BlockSize - 1) / BlockSize;
	DispatchSize.Y = (FOceanRenderer::FFT_Size + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);

	// Do IFFT Horizontal
	int32 PingPongIndex = 0;
	for (int32 stage = 0; stage < Stages; stage++)
	{
		RHI->SetComputeConstantBuffer(0, InfoUniformHorizontal[stage]->UniformBuffer);
		RHI->SetComputeResourceTable(1, ResourceTables[PingPongIndex]);

		RHI->DispatchCompute(vector3di(BlockSize, BlockSize, 1), DispatchSize);
		PingPongIndex = (PingPongIndex + 1) % 2;
	}

	// Do IFFT Vertical
	for (int32 stage = 0; stage < Stages; stage++)
	{
		RHI->SetComputeConstantBuffer(0, InfoUniformVertical[stage]->UniformBuffer);
		RHI->SetComputeResourceTable(1, ResourceTables[PingPongIndex]);

		RHI->DispatchCompute(vector3di(BlockSize, BlockSize, 1), DispatchSize);
		PingPongIndex = (PingPongIndex + 1) % 2;
	}
}