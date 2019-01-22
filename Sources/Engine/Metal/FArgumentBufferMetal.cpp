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
	{
        TI_ASSERT(0);
        // Argument buffer implementation
        
        // Shader reflection to get Bind index
	}

	FArgumentBufferMetal::~FArgumentBufferMetal()
	{
		TI_ASSERT(IsRenderThread());
	}
}

#endif	// COMPILE_WITH_RHI_METAL
