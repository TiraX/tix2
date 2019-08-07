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
	FArgumentBufferDx12::FArgumentBufferDx12(int32 ReservedTextures)
		: FArgumentBuffer(ReservedTextures)
	{
	}

	FArgumentBufferDx12::~FArgumentBufferDx12()
	{
		TI_ASSERT(IsRenderThread());
		UniformBuffer = nullptr;
		TextureResourceTable = nullptr;
	}
}

#endif	// COMPILE_WITH_RHI_DX12