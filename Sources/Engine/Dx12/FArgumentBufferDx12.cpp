/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FArgumentBufferDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FArgumentBufferDx12::FArgumentBufferDx12(FShaderPtr InShader)
		: FArgumentBuffer(InShader)
	{
		// Set Index directly
		TI_TODO("Figure out a way to get Argument BindIndex");
		Index = 3;
	}

	FArgumentBufferDx12::~FArgumentBufferDx12()
	{
		TI_ASSERT(IsRenderThread());
		UniformBuffer = nullptr;
		TextureResourceTable = nullptr;
	}
}

#endif	// COMPILE_WITH_RHI_DX12