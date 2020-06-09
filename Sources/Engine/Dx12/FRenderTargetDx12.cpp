/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FRenderTargetDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FRenderTargetDx12::FRenderTargetDx12(int32 W, int32 H)
		: FRenderTarget(W, H)
	{
	}

	FRenderTargetDx12::~FRenderTargetDx12()
	{
		TI_ASSERT(IsRenderThread());
	}
}

#endif	// COMPILE_WITH_RHI_DX12