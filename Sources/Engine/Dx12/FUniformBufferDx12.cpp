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
	FUniformBufferDx12::FUniformBufferDx12(uint32 InStructureSizeInBytes, uint32 Elements, uint32 InFlag)
		: FUniformBuffer(InStructureSizeInBytes, Elements, InFlag)
	{
	}

	FUniformBufferDx12::~FUniformBufferDx12()
	{
		TI_ASSERT(IsRenderThread());
	}
}

#endif	// COMPILE_WITH_RHI_DX12