/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRenderResourceHeap;
	class FRenderResourceTable : public FRenderResource
	{
	public:
		FRenderResourceTable(uint32 InSize);
		FRenderResourceTable(FRenderResourceHeap * InHeap, uint32 InStart, uint32 InSize);
		~FRenderResourceTable();

		TI_API void PutConstantBufferInTable(FUniformBufferPtr InUniformBuffer, uint32 Index);
		TI_API void PutTextureInTable(FTexturePtr InTexture, uint32 Index);
		TI_API void PutRWTextureInTable(FTexturePtr InTexture, uint32 MipLevel, uint32 Index);
		TI_API void PutUniformBufferInTable(FUniformBufferPtr InBuffer, uint32 Index);
		TI_API void PutRWUniformBufferInTable(FUniformBufferPtr InBuffer, uint32 Index);
		TI_API void PutMeshBufferInTable(FMeshBufferPtr InBuffer, int32 VBIndex, int32 IBIndex);
		TI_API void PutInstanceBufferInTable(FInstanceBufferPtr InBuffer, uint32 Index);
		TI_API void PutRTColorInTable(FTexturePtr InTexture, uint32 Index);
		TI_API void PutRTDepthInTable(FTexturePtr InTexture, uint32 Index);
		TI_API void PutTopLevelAccelerationStructureInTable(FTopLevelAccelerationStructurePtr InTLAS, uint32 Index);

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

		TI_API E_RENDER_RESOURCE_HEAP_TYPE GetHeapType() const;
	protected:
		E_RENDER_RESOURCE_HEAP_TYPE HeapType;
		uint32 Start;
		uint32 Size;

		friend class FRenderResourceHeap;
	};
}