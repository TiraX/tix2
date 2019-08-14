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
        RenderPipelineState = nil;
        DepthState = nil;
        ComputePipelineState = nil;
	}

	FPipelineMetal::~FPipelineMetal()
	{
		TI_ASSERT(IsRenderThread());
        RenderPipelineState = nil;
        DepthState = nil;
        ComputePipelineState = nil;
	}
}

#endif	// COMPILE_WITH_RHI_METAL
