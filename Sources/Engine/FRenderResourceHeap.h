/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
#define LIGHTS_IN_CBV_SRV_UAV_HEAP 64
	
	class FRenderResourceHeap
	{
	public:
		FRenderResourceHeap();
		~FRenderResourceHeap();

		void Create(E_RENDER_RESOURCE_HEAP_TYPE HeapType, uint32 HeapSize, uint32 HeapOffset);

		uint32 AllocateSlot();
		void RecallSlot(uint32 HeapSlot);
		int32 GetHeapSize() const
		{
			return Size;
		}

	private:
		E_RENDER_RESOURCE_HEAP_TYPE HeapType;
		TVector<uint32> AvaibleHeapSlots;
		uint32 Allocated;
		uint32 Size;
		uint32 Offset;
	};
}