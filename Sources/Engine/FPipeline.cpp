/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FPipeline.h"

namespace tix
{
	FPipeline::FPipeline(FShaderPtr InShader)
		: Shader(InShader)
	{
	}

	FPipeline::~FPipeline()
	{
	}
}