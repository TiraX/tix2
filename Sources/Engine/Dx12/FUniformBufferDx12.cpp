/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FUniformBufferDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FUniformBufferDx12::FUniformBufferDx12(uint32 InStructSize)
		: FUniformBuffer(InStructSize)
	{
	}

	FUniformBufferDx12::~FUniformBufferDx12()
	{
		TI_ASSERT(IsRenderThread());
		ConstantBuffer = nullptr;
	}
}

#endif	// COMPILE_WITH_RHI_DX12