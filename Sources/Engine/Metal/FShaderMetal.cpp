/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIMetal.h"
#include "FShaderMetal.h"

#if COMPILE_WITH_RHI_METAL

namespace tix
{
    FShaderMetal::FShaderMetal(const TString& ComputeShaderName)
        : FShader(ComputeShaderName)
    {
        VertexComputeProgram = nil;
        FragmentProgram = nil;
    }
    
	FShaderMetal::FShaderMetal(const TShaderNames& InNames)
		: FShader(InNames)
	{
        VertexComputeProgram = nil;
        FragmentProgram = nil;
	}

	FShaderMetal::~FShaderMetal()
	{
        TI_ASSERT(IsRenderThread());
        
        VertexComputeProgram = nil;
        FragmentProgram = nil;
	}
}

#endif	// COMPILE_WITH_RHI_METAL
