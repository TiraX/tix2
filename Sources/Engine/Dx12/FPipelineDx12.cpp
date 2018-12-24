/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FPipelineDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FPipelineDx12::FPipelineDx12()
	{
	}

	FPipelineDx12::~FPipelineDx12()
	{
		TI_ASSERT(IsRenderThread());
		PipelineState = nullptr;
	}
}

#endif	// COMPILE_WITH_RHI_DX12