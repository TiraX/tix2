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
	FShaderDx12::FShaderDx12(const TShaderNames& InNames)
		: FShader(InNames)
	{
	}

	FShaderDx12::~FShaderDx12()
	{
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