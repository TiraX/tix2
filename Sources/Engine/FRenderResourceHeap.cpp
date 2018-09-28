/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRenderResourceHeap.h"

namespace tix
{
	static const int32 MaxDescriptorCount[EHT_COUNT] =
	{
		1024,	//EHT_CBV_SRV_UAV,
		512,	//EHT_SAMPLER,
		128,	//EHT_RTV,
		32,		//EHT_DSV,
	};

	FRenderResourceHeap::FRenderResourceHeap()
		: HeapType(EHT_NONE)
		, Allocated(0)
		, Size(0)
		, Offset(0)
	{
	}

	FRenderResourceHeap::~FRenderResourceHeap()
	{
	}

	void FRenderResourceHeap::Create(E_RENDER_RESOURCE_HEAP_TYPE InHeapType, uint32 HeapSize, uint32 HeapOffset)
	{
		HeapType = InHeapType;
		Size = HeapSize;
		Offset = HeapOffset;
	}

	uint32 FRenderResourceHeap::AllocateSlot()
	{
		if (AvaibleHeapSlots.size() > 0)
		{
			uint32 SlotIndex = AvaibleHeapSlots.back();
			AvaibleHeapSlots.pop_back();
			return SlotIndex;
		}
		uint32 Result = Allocated + Offset;
		++Allocated;
		TI_ASSERT(Allocated < Size);
		return Result;
	}

	void FRenderResourceHeap::RecallSlot(uint32 HeapSlot)
	{
		AvaibleHeapSlots.push_back(HeapSlot);
	}
}