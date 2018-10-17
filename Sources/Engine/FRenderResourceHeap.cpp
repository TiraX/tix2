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

	FRenderResourceTable FRenderResourceHeap::AllocateTable(uint32 TableSize)
	{
		TI_ASSERT(IsRenderThread());
		TVector<uint32>& Avaibles = AvaibleHeapTables[TableSize];

		if (Avaibles.size() > 0)
		{
			uint32 StartIndex = Avaibles.back();
			Avaibles.pop_back();
			return FRenderResourceTable(this, StartIndex, TableSize);
		}
		uint32 Result = Allocated + Offset;
		Allocated += TableSize;
		TI_ASSERT(Allocated <= Size);
		return FRenderResourceTable(this, Result, TableSize);
	}

	void FRenderResourceHeap::RecallTable(const FRenderResourceTable& Table)
	{
		TI_ASSERT(IsRenderThread());
		TVector<uint32>& Avaibles = AvaibleHeapTables[Table.GetTableSize()];
		Avaibles.push_back(Table.GetStartIndex());
	}
}