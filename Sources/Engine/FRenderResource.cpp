/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRenderResource.h"

namespace tix
{
	FRenderResourceInHeap::FRenderResourceInHeap(E_RENDER_RESOURCE_HEAP_TYPE InHeapType)
		: HeapType(InHeapType)
		, HeapSlot(uint32(-1))
	{
	}

	FRenderResourceInHeap::~FRenderResourceInHeap()
	{
		TI_ASSERT(IsRenderThread());
		if (HeapSlot != uint32(-1))
		{
			FRHI::Get()->RecallHeapSlot(HeapType, HeapSlot);
		}
	}

	void FRenderResourceInHeap::InitRenderResourceHeapSlot()
	{
		TI_ASSERT(IsRenderThread());
		TI_ASSERT(HeapType != EHT_NONE);
		TI_ASSERT(HeapSlot == uint32(-1));
		HeapSlot = FRHI::Get()->AllocateHeapSlot(HeapType);
	}
}