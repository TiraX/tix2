/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "CameraVolumeLut.h"

FCameraVolumeLutCS::FCameraVolumeLutCS()
	: FComputeTask("S_CameraVolumeLutCS")
{
}

FCameraVolumeLutCS::~FCameraVolumeLutCS()
{
}

void FCameraVolumeLutCS::PrepareResources(FRHI * RHI)
{
	int32 RTW = RHI->GetViewport().Width;
	int32 RTH = RHI->GetViewport().Height;
}

void FCameraVolumeLutCS::UpdataComputeParams(
	FRHI * RHI
)
{
}

void FCameraVolumeLutCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 8;
}