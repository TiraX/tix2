/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "MeanIllumLutCS.h"

FMeanIllumLutCS::FMeanIllumLutCS()
	: FComputeTask("S_MeanIllumLutCS")
{
}

FMeanIllumLutCS::~FMeanIllumLutCS()
{
}

void FMeanIllumLutCS::PrepareResources(FRHI* RHI)
{
	int32 RTW = RHI->GetViewport().Width;
	int32 RTH = RHI->GetViewport().Height;
}

void FMeanIllumLutCS::UpdataComputeParams(
	FRHI* RHI
)
{
}

void FMeanIllumLutCS::Run(FRHI* RHI)
{
	const uint32 BlockSize = 8;
}