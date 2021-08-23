/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FShaderDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FShaderDx12::FShaderDx12(const TString& InShaderName, E_SHADER_TYPE InType)
		: FShader(InShaderName, InType)
	{
	}

	FShaderDx12::FShaderDx12(const TShaderNames& RenderShaderNames)
		: FShader(RenderShaderNames)
	{
	}

	FShaderDx12::~FShaderDx12()
	{
		TI_ASSERT(IsRenderThread());
		ShaderBinding = nullptr;
	}

	void FShaderDx12::ReleaseShaderCode()
	{
		for (int32 s = 0; s < ESS_COUNT; ++s)
		{
			ShaderCodes[s].Destroy();
		}
	}
}

#endif	// COMPILE_WITH_RHI_DX12