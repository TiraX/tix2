/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FShaderDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FShaderDx12::FShaderDx12(const TString& InShaderName)
		: FShader(InShaderName)
	{
	}

	FShaderDx12::~FShaderDx12()
	{
	}

	void FShaderDx12::ReleaseShaderCode()
	{
		Shader.Destroy();
	}
}

#endif	// COMPILE_WITH_RHI_DX12