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
	FRenderTargetDx12::FRenderTargetDx12(int32 W, int32 H)
		: FRenderTarget(ERF_Dx12, W, H)
	{
		for (int32 i = 0; i < ERTC_COUNT; ++i)
		{
			RTColorDescriptor[i].ptr = 0;
		}
		RTDSDescriptor.ptr = 0;
	}

	FRenderTargetDx12::~FRenderTargetDx12()
	{
		Destroy();
	}

	void FRenderTargetDx12::Destroy()
	{
		TI_ASSERT(IsRenderThread());
		FRHIDx12 * RHIDx12 = static_cast<FRHIDx12*>(FRHI::Get());
		for (int32 i = 0; i < ERTC_COUNT; ++i)
		{
			if (RTColorDescriptor[i].ptr != 0)
			{
				RHIDx12->RecallDescriptor(EHT_RTV, RTColorDescriptor[i]);
				RTColorDescriptor[i].ptr = 0;
			}
		}
		if (RTDSDescriptor.ptr != 0)
		{
			RHIDx12->RecallDescriptor(EHT_DSV, RTDSDescriptor);
			RTDSDescriptor.ptr = 0;
		}
	}
}

#endif	// COMPILE_WITH_RHI_DX12