/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FRenderResourceTable.h"

namespace tix
{
#define LIGHTS_IN_CBV_SRV_UAV_HEAP 64
	
	class FRenderResourceHeap
	{
	public:
		FRenderResourceHeap();
		~FRenderResourceHeap();

		void Create(E_RENDER_RESOURCE_HEAP_TYPE HeapType, uint32 HeapSize, uint32 HeapOffset);

		FRenderResourceTable AllocateTable(uint32 Size);

		void RecallTable(const FRenderResourceTable& Table);

		E_RENDER_RESOURCE_HEAP_TYPE GetHeapType() const
		{
			return HeapType;
		}
		int32 GetHeapSize() const
		{
			return Size;
		}

	private:

	private:
		E_RENDER_RESOURCE_HEAP_TYPE HeapType;
		THMap<uint32, TVector<uint32>> AvaibleHeapTables;
		uint32 Allocated;
		uint32 Size;
		uint32 Offset;

		friend class FRenderResourceTable;
	};
}