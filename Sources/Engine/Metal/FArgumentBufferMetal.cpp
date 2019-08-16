/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIMetal.h"
#include "FArgumentBufferMetal.h"

#if COMPILE_WITH_RHI_METAL

namespace tix
{
	FArgumentBufferMetal::FArgumentBufferMetal(int32 ReservedSlots)
		: FArgumentBuffer(ReservedSlots)
	{
        ArgumentBuffer = nil;
	}

	FArgumentBufferMetal::~FArgumentBufferMetal()
	{
		TI_ASSERT(IsRenderThread());
        ArgumentBuffer = nil;
	}
}

#endif	// COMPILE_WITH_RHI_METAL
