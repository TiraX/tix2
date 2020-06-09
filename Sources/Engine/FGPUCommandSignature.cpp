/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FGPUCommandSignature.h"

namespace tix
{
	FGPUCommandSignature::FGPUCommandSignature(FPipelinePtr InPipeline, const TVector<E_GPU_COMMAND_TYPE>& InCommandStructure)
		: FRenderResource(RRT_GPU_COMMAND_SIGNATURE)
		, Pipeline(InPipeline)
		, CommandStructure(InCommandStructure)
	{
	}

	FGPUCommandSignature::~FGPUCommandSignature()
	{
	}
}
