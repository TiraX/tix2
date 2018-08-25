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
		for (int32 i = 0; i < ERTC_COUNT; ++i)
		{
			RTColorDescriptor[i] = uint32(-1);
		}
		RTDSDescriptor = uint32(-1);
	}

	FRenderTargetDx12::~FRenderTargetDx12()
	{
		Destroy();
	}

	void FRenderTargetDx12::Destroy()
	{
		TI_ASSERT(IsRenderThread());
		for (int32 i = 0; i < ERTC_COUNT; ++i)
		{
			if (RTColorDescriptor[i] != uint32(-1))
			{
				TI_ASSERT(0);
				TI_TODO("Recall rtv descriptor");
				RTColorDescriptor[i] = uint32(-1);
			}
		}
		TI_TODO("Recall dsv descriptor");
		RTDSDescriptor = uint32(-1);
	}
}

#endif	// COMPILE_WITH_RHI_DX12