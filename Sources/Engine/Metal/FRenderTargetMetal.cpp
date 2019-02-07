/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIMetal.h"
#include "FRenderTargetMetal.h"

#if COMPILE_WITH_RHI_METAL

namespace tix
{
    FRenderTargetMetal::FRenderTargetMetal(int32 W, int32 H)
        : FRenderTarget(W, H)
    {
        RenderPassDesc = nil;
    }

    FRenderTargetMetal::~FRenderTargetMetal()
    {
        TI_ASSERT(IsRenderThread());
        RenderPassDesc = nil;
    }
}

#endif	// COMPILE_WITH_RHI_METAL
