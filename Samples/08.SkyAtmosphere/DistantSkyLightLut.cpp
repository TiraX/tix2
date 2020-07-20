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
	int32 RTW = RHI->GetViewport().Width;
	int32 RTH = RHI->GetViewport().Height;
}

void FDistantSkyLightLutCS::UpdataComputeParams(
	FRHI* RHI
)
{
}

void FDistantSkyLightLutCS::Run(FRHI* RHI)
{
	const uint32 BlockSize = 8;
}