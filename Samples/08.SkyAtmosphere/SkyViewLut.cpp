/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SkyViewLut.h"

FSkyViewLutCS::FSkyViewLutCS()
	: FComputeTask("S_SkyViewLutCS")
{
}

FSkyViewLutCS::~FSkyViewLutCS()
{
}

void FSkyViewLutCS::PrepareResources(FRHI* RHI)
{
	int32 RTW = RHI->GetViewport().Width;
	int32 RTH = RHI->GetViewport().Height;
}

void FSkyViewLutCS::UpdataComputeParams(
	FRHI* RHI
)
{
}

void FSkyViewLutCS::Run(FRHI* RHI)
{
	const uint32 BlockSize = 8;
}