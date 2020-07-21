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
	//DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, TransmittanceLutSizeAndInv)	// xy = Size; zw = InvSize;
	//DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, RadiusRange)	// x = TopRadiusKm; y = BottomRadiusKm; z = sqrt(x * x - y * y); w = y * y;
	//DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MieRayleigh)	// x = MiePhaseG; y = MieDensityExpScale; z = RayleighDensityExpScale; w = 1;
	//DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Params1)	// x = TransmittanceSampleCount; y = ViewPreExposure; z = AbsorptionDensity0LayerWidth; w = 1
	//DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, AbsorptionDensity01MA)
	//DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MieScattering)
	//DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MieAbsorption)
	//DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MieExtinction)
	//DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, RayleighScattering)
	//DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, AbsorptionExtinction)
	//DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, GroundAlbedo)
	AtomsphereParam = ti_new FAtomsphereParam;
	AtomsphereParam->UniformBufferData[0].TransmittanceLutSizeAndInv = FFloat4(float(LUT_W), float(LUT_H), 1.f / float(LUT_W), 1.f / float(LUT_H));
	const float TopRadiusKm = 6420.f;
	const float BottomRadiusKm = 6360.f;
	AtomsphereParam->UniformBufferData[0].RadiusRange = FFloat4(TopRadiusKm, BottomRadiusKm, sqrt(TopRadiusKm * TopRadiusKm - BottomRadiusKm * BottomRadiusKm), BottomRadiusKm * BottomRadiusKm);
	AtomsphereParam->UniformBufferData[0].MieRayleigh = FFloat4(0.8f, -0.83333f, -0.125f, 1.f);
	AtomsphereParam->UniformBufferData[0].Params1 = FFloat4(10.f, 1.f, 25.f, 1.f);
	AtomsphereParam->UniformBufferData[0].AbsorptionDensity01MA = FFloat4(0.06667f, -0.6667f, -0.06667f, 2.66667f);
	AtomsphereParam->UniformBufferData[0].MieScattering = FFloat4(0.004f, 0.004f, 0.004f, 0.004f);
	AtomsphereParam->UniformBufferData[0].MieAbsorption = FFloat4(0.00044f, 0.00044f, 0.00044f, 0.00044f);
	AtomsphereParam->UniformBufferData[0].MieExtinction = FFloat4(0.00444f, 0.00444f, 0.00444f, 0.00444f);
	AtomsphereParam->UniformBufferData[0].RayleighScattering = FFloat4(0.0058f, 0.01356f, 0.0331f, 1.f);
	AtomsphereParam->UniformBufferData[0].AbsorptionExtinction = FFloat4(0.00065f, 0.00188f, 0.00008f, 1.f);
	AtomsphereParam->UniformBufferData[0].GroundAlbedo = FFloat4(0.40198f, 0.40198f, 0.40198f, 1.f);
	AtomsphereParam->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	TTextureDesc LutDesc;
	LutDesc.Type = ETT_TEXTURE_2D;
	TI_TODO("Use R11G11B10 Format for optimization");
	LutDesc.Format = EPF_RGBA32F;
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

void FTransmittanceLutCS::Run(FRHI* RHI)
{
	const uint32 BlockSize = 8;
	int32 RTW = RHI->GetViewport().Width;
	int32 RTH = RHI->GetViewport().Height;

	vector3di DispatchSize = vector3di(1, 1, 1);
	DispatchSize.X = (LUT_W + BlockSize - 1) / BlockSize;
	DispatchSize.Y = (LUT_H + BlockSize - 1) / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, AtomsphereParam->UniformBuffer);
	RHI->SetComputeResourceTable(1, ResourceTable);

	RHI->DispatchCompute(vector3di(BlockSize, BlockSize, 1), DispatchSize);
}