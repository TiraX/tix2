/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRenderResourceHeap;
	class FRenderResourceTable
	{
	public:
		FRenderResourceTable();
		FRenderResourceTable(FRenderResourceHeap * InHeap, uint32 InStart, uint32 InSize);
		~FRenderResourceTable();

		virtual void PutUniformBufferInTable(FUniformBufferPtr InUniformBuffer, uint32 Index);
		virtual void PutTextureInTable(FTexturePtr InTexture, uint32 Index);
		virtual void PutRTColorInTable(FTexturePtr InTexture, uint32 Index);
		virtual void PutRTDepthInTable(FTexturePtr InTexture, uint32 Index);

		uint32 GetStartIndex() const
		{
			return Start;
		}

		uint32 GetIndexAt(uint32 Index) const
		{
			TI_ASSERT(Index < Size);
			return Start + Index;
		}

		uint32 GetTableSize() const
		{
			return Size;
		}

		E_RENDER_RESOURCE_HEAP_TYPE GetHeapType() const;
	protected:
		FRenderResourceHeap* Heap;
		uint32 Start;
		uint32 Size;
	};
}