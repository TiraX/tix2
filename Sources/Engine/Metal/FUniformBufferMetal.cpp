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
    
    TStreamPtr FUniformBufferMetal::ReadBufferData()
    {
        if (Buffer != nil)
        {
            TStreamPtr Data = ti_new TStream((int32)Buffer.length);
            Data->Put([Buffer contents], (int32)Buffer.length);
            
            return Data;
        }
        return nullptr;
    }
}

#endif	// COMPILE_WITH_RHI_METAL
