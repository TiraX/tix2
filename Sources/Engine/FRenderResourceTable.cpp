/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRenderResourceTable.h"

namespace tix
{
	FRenderResourceTable::FRenderResourceTable()
		: Heap(nullptr)
		, Start(uint32(-1))
		, Size(0)
	{
	}

	FRenderResourceTable::FRenderResourceTable(FRenderResourceHeap * InHeap, uint32 InStart, uint32 InSize)
		: Heap(InHeap)
		, Start(InStart)
		, Size(InSize)
	{
	}

	FRenderResourceTable::~FRenderResourceTable()
	{
		Heap->RecallTable(*this);
	}

	void FRenderResourceTable::PutUniformBufferInTable(FUniformBufferPtr InUniformBuffer, uint32 Index)
	{
		TI_ASSERT(Index < Size);
		FRHI::Get()->PutUniformBufferInHeap(InUniformBuffer, Heap->GetHeapType(), Start + Index);
	}

	void FRenderResourceTable::PutTextureInTable(FTexturePtr InTexture, uint32 Index)
	{
		TI_ASSERT(Index < Size);
		FRHI::Get()->PutTextureInHeap(InTexture, Heap->GetHeapType(), Start + Index);
	}

	E_RENDER_RESOURCE_HEAP_TYPE FRenderResourceTable::GetHeapType() const
	{
		return Heap->GetHeapType();
	}
}