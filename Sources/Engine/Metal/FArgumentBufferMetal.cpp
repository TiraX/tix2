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
	FArgumentBufferMetal::FArgumentBufferMetal(FShaderPtr InShader)
		: FArgumentBuffer(InShader)
        , ArgumentBindIndex(-1)
	{
        ArgumentBuffer = nil;
	}

	FArgumentBufferMetal::~FArgumentBufferMetal()
	{
		TI_ASSERT(IsRenderThread());
        ArgumentBuffer = nil;
        for (auto& Buffer : Buffers)
        {
            Buffer = nil;
        }
        Buffers.clear();
        for (auto& Texture : Textures)
        {
            Texture = nil;
        }
        Textures.clear();
	}
}

#endif	// COMPILE_WITH_RHI_METAL
