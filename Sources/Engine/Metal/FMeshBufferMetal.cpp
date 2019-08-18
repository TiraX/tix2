/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIMetal.h"
#include "FMeshBufferMetal.h"

#if COMPILE_WITH_RHI_METAL

namespace tix
{
	FMeshBufferMetal::FMeshBufferMetal()
	{
        VertexBuffer = nil;
        IndexBuffer = nil;
	}

	FMeshBufferMetal::~FMeshBufferMetal()
	{
		TI_ASSERT(IsRenderThread());
        VertexBuffer = nil;
        IndexBuffer = nil;
	}
    
    /////////////////////////////////////////////////////////////
    FInstanceBufferMetal::FInstanceBufferMetal()
    {
    }
    
    FInstanceBufferMetal::~FInstanceBufferMetal()
    {
        TI_ASSERT(IsRenderThread());
        InstanceBuffer = nullptr;
    }
}

#endif	// COMPILE_WITH_RHI_METAL
