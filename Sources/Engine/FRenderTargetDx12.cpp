/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FRenderTargetDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FRenderTargetDx12::FRenderTargetDx12()
		: FRenderTarget(ERF_Dx12)
	{
	}

	FRenderTargetDx12::~FRenderTargetDx12()
	{
		Destroy();
	}

	void FRenderTargetDx12::Destroy()
	{
		TI_ASSERT(IsRenderThread());
		TI_ASSERT(0);
	}
}

#endif	// COMPILE_WITH_RHI_DX12