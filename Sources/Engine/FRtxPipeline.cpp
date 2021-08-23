/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRtxPipeline.h"

namespace tix
{
	FRtxPipeline::FRtxPipeline(FShaderPtr InShader)
		: FRenderResource(RRT_RTX_PIPELINE)
		, ShaderLib(InShader)
	{
	}

	FRtxPipeline::~FRtxPipeline()
	{
	}
}