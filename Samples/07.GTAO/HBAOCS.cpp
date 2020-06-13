/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "HBAOCS.h"

FHBAOCS::FHBAOCS()
	: FComputeTask("S_HBAOCS")
{
}

FHBAOCS::~FHBAOCS()
{
}

void FHBAOCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

}

void FHBAOCS::UpdataComputeParams(
	FRHI * RHI
)
{
}

void FHBAOCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
}