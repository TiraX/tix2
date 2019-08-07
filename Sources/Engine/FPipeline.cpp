/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FPipeline.h"

namespace tix
{
	FPipeline::FPipeline(FShaderPtr InShader)
		: FRenderResource(RRT_PIPELINE)
		, Shader(InShader)
	{
	}

	FPipeline::~FPipeline()
	{
	}

	///////////////////////////////////////////////////////////////////////////

	//FRenderPipeline::FRenderPipeline(FShaderPtr InShader)
	//	: FPipeline(InShader)
	//{
	//}

	//FRenderPipeline::~FRenderPipeline()
	//{
	//}

	///////////////////////////////////////////////////////////////////////////

	//FComputePipeline::FComputePipeline(FShaderPtr InShader)
	//	: FPipeline(InShader)
	//{
	//}

	//FComputePipeline::~FComputePipeline()
	//{
	//}
}