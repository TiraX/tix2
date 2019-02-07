/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIMetal.h"
#include "FUniformBufferMetal.h"

#if COMPILE_WITH_RHI_METAL

namespace tix
{
	FUniformBufferMetal::FUniformBufferMetal(uint32 InStructSize)
		: FUniformBuffer(InStructSize)
	{
        ConstantBuffer = nil;
	}

	FUniformBufferMetal::~FUniformBufferMetal()
	{
		TI_ASSERT(IsRenderThread());
		ConstantBuffer = nil;
	}
}

#endif	// COMPILE_WITH_RHI_METAL
