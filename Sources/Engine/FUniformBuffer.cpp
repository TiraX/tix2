/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FUniformBuffer.h"

namespace tix
{
	FUniformBuffer::FUniformBuffer(E_RENDER_RESOURCE_HEAP_TYPE HeapType, uint32 InStructSize)
		: FRenderResourceInHeap(HeapType)
		, StructSize(InStructSize)
	{
	}

	FUniformBuffer::~FUniformBuffer()
	{
	}
}