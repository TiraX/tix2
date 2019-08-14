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
	FUniformBufferMetal::FUniformBufferMetal(uint32 InStructureSizeInBytes, uint32 Elements, uint32 InFlag)
		: FUniformBuffer(InStructureSizeInBytes, Elements, InFlag)
	{
        Buffer = nil;
	}

	FUniformBufferMetal::~FUniformBufferMetal()
	{
		TI_ASSERT(IsRenderThread());
		Buffer = nil;
	}
}

#endif	// COMPILE_WITH_RHI_METAL
