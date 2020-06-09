/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FShader.h"

namespace tix
{
	FShader::FShader(const TString& ComputeShaderName)
		: FRenderResource(RRT_SHADER)
		, Type(EST_COMPUTE)
	{
		ShaderNames.ShaderNames[0] = ComputeShaderName;
	}

	FShader::FShader(const TShaderNames& RenderShaderNames)
		: FRenderResource(RRT_SHADER)
		, ShaderNames(RenderShaderNames)
		, Type(EST_RENDER)
	{
	}

	FShader::~FShader()
	{
        TI_ASSERT(IsRenderThread());
	}
}
