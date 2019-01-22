/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FShader.h"

namespace tix
{
	FShader::FShader(const TShaderNames& InNames)
		: ShaderNames(InNames)
	{
	}

	FShader::~FShader()
	{
        TI_ASSERT(IsRenderThread());
	}
}
