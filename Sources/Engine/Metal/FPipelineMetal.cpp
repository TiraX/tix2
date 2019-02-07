/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIMetal.h"
#include "FPipelineMetal.h"

#if COMPILE_WITH_RHI_METAL

namespace tix
{
	FPipelineMetal::FPipelineMetal(FShaderPtr InShader)
        : FPipeline(InShader)
	{
        PipelineState = nil;
        DepthState = nil;
	}

	FPipelineMetal::~FPipelineMetal()
	{
		TI_ASSERT(IsRenderThread());
        PipelineState = nil;
        DepthState = nil;
	}
}

#endif	// COMPILE_WITH_RHI_METAL
