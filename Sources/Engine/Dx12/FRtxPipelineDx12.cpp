/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FRtxPipelineDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FRtxPipelineDx12::FRtxPipelineDx12(FShaderPtr InShader)
		: FRtxPipeline(InShader)
	{
	}

	FRtxPipelineDx12::~FRtxPipelineDx12()
	{
		TI_ASSERT(IsRenderThread());
		StateObject = nullptr;
	}
}

#endif	// COMPILE_WITH_RHI_DX12