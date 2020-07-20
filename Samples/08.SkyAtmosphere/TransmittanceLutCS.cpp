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
	int32 RTW = RHI->GetViewport().Width;
	int32 RTH = RHI->GetViewport().Height;
}

void FTransmittanceLutCS::UpdataComputeParams(
	FRHI* RHI
)
{
}

void FTransmittanceLutCS::Run(FRHI* RHI)
{
	const uint32 BlockSize = 8;
}