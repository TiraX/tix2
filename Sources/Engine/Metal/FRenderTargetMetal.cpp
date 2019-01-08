/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIMetal.h"
#include "FRenderTargetMetal.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	//FRenderTargetDx12::FRenderTargetDx12(int32 W, int32 H)
	//	: FRenderTarget(W, H)
	//{
	//	for (int32 i = 0; i < ERTC_COUNT; ++i)
	//	{
	//		RTColorDescriptor[i].ptr = 0;
	//	}
	//	RTDSDescriptor.ptr = 0;
	//}

	//FRenderTargetDx12::~FRenderTargetDx12()
	//{
	//	Destroy();
	//}

	//void FRenderTargetDx12::Destroy()
	//{
	//	TI_ASSERT(IsRenderThread());
	//}
}

#endif	// COMPILE_WITH_RHI_DX12
