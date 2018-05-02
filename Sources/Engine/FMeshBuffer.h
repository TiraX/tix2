/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FMeshBuffer;
	typedef TI_INTRUSIVE_PTR(FMeshBuffer) FMeshBufferPtr;

	class FMeshBuffer : public IReferenceCounted
	{
	public:
		FMeshBuffer(E_MB_TYPES type);
		virtual ~FMeshBuffer();

	public:
		virtual void	SetVertexStreamData(uint32 format,
			void* vertex_data, int32 vertex_count,
			E_INDEX_TYPE index_type, void* index_data, int32 index_count);

		int32	GetVerticesCount()
		{
			return VsDataCount;
		}

		int32 GetIndicesCount()
		{
			return PsDataCount;
		}

		E_MB_TYPES	GetType()
		{
			return Type;
		}

		E_PRIMITIVE_TYPE GetPrimitiveType()
		{
			return PrimitiveType;
		}

		E_INDEX_TYPE GetIndexType()
		{
			return IndexType;
		}

		void SetPrimitiveType(E_PRIMITIVE_TYPE type)
		{
			PrimitiveType = type;
		}

		uint32 GetVSFormat()
		{
			return VsFormat;
		}

		const aabbox3df& GetBBox()
		{
			return BBox;
		}

		void SetBBox(const aabbox3df& bbox)
		{
			BBox = bbox;
		}

		const void* GetVSData() const
		{
			return VsData;
		}

		const void* GetPSData() const
		{
			return PsData;
		}
	protected:
		virtual void CreateHardwareBuffer() = 0;

	protected:
		E_MB_TYPES			Type;
		E_PRIMITIVE_TYPE	PrimitiveType;
		int32				Usage;
		aabbox3df			BBox;

		uint32				MeshFlag;

		uint8*				VsData;
		int32				VsDataCount;

		E_INDEX_TYPE		IndexType;
		uint8*				PsData;
		int32				PsDataCount;

		uint32				VsFormat;
	};
}
