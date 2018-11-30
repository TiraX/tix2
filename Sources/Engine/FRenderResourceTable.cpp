/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRenderResourceTable.h"

namespace tix
{
	FRenderResourceTable::FRenderResourceTable(uint32 InSize)
		: HeapType(EHT_NONE)
		, Start(uint32(-1))
		, Size(InSize)
	{
	}

	FRenderResourceTable::FRenderResourceTable(FRenderResourceHeap * InHeap, uint32 InStart, uint32 InSize)
		: HeapType(InHeap->GetHeapType())
		, Start(InStart)
		, Size(InSize)
	{
	}

	FRenderResourceTable::~FRenderResourceTable()
	{
		TI_ASSERT(IsRenderThread());
		FRHI::Get()->GetRenderResourceHeap(HeapType).RecallTable(*this);
	}

	void FRenderResourceTable::PutUniformBufferInTable(FUniformBufferPtr InUniformBuffer, uint32 Index)
	{
		TI_ASSERT(HeapType == EHT_UNIFORMBUFFER);
		TI_ASSERT(Index < Size);
		FRHI::Get()->PutUniformBufferInHeap(InUniformBuffer, HeapType, Start + Index);
	}

	void FRenderResourceTable::PutTextureInTable(FTexturePtr InTexture, uint32 Index)
	{
		TI_ASSERT(HeapType == EHT_TEXTURE);
		TI_ASSERT(Index < Size);
		FRHI::Get()->PutTextureInHeap(InTexture, HeapType, Start + Index);
	}

	void FRenderResourceTable::PutRTColorInTable(FTexturePtr InTexture, uint32 Index)
	{
		TI_ASSERT(HeapType == EHT_RENDERTARGET);
		TI_ASSERT(Index < Size);
		FRHI::Get()->PutRTColorInHeap(InTexture, Start + Index);
	}

	void FRenderResourceTable::PutRTDepthInTable(FTexturePtr InTexture, uint32 Index)
	{
		TI_ASSERT(HeapType == EHT_DEPTHSTENCIL);
		TI_ASSERT(Index < Size);
		FRHI::Get()->PutRTDepthInHeap(InTexture, Start + Index);
	}

	E_RENDER_RESOURCE_HEAP_TYPE FRenderResourceTable::GetHeapType() const
	{
		return HeapType;
	}
}