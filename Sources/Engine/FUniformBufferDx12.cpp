/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FUniformBufferDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FUniformBufferDx12::FUniformBufferDx12(E_RENDER_RESOURCE_HEAP_TYPE InHeap, uint32 InStructSize)
		: FUniformBuffer(InHeap, InStructSize)
	{
	}

	FUniformBufferDx12::~FUniformBufferDx12()
	{
		Destroy();
	}

	void FUniformBufferDx12::Destroy()
	{
		TI_ASSERT(IsRenderThread());
		ConstantBuffer = nullptr;
	}
}

#endif	// COMPILE_WITH_RHI_DX12