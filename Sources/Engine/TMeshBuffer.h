/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	// TMeshBuffer, hold mesh vertex and index data memory in game thread
	class TI_API TMeshBuffer : public TResource
	{
	public:
		TMeshBuffer();
		~TMeshBuffer();

		static const int32 SemanticSize[ESSI_TOTAL];
		static const int8* SemanticName[ESSI_TOTAL];
		static const int32 SemanticIndex[ESSI_TOTAL];
	public:
		FMeshBufferPtr MeshBufferResource;

		static uint32 GetStrideFromFormat(uint32 Format);
		static TVector<E_MESH_STREAM_INDEX> GetSteamsFromFormat(uint32 Format);

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		void SetVertexStreamData(
			uint32 InFormat,
			const void* InVertexData, uint32 InVertexCount,
			E_INDEX_TYPE InIndexType,
			const void* InIndexData, uint32 InIndexCount);

		uint32 GetVerticesCount() const
		{
			return VsDataCount;
		}

		uint32 GetIndicesCount() const
		{
			return PsDataCount;
		}

		E_PRIMITIVE_TYPE GetPrimitiveType() const
		{
			return PrimitiveType;
		}

		E_INDEX_TYPE GetIndexType() const
		{
			return IndexType;
		}

		void SetPrimitiveType(E_PRIMITIVE_TYPE type)
		{
			PrimitiveType = type;
		}

		uint32 GetVSFormat() const
		{
			return VsFormat;
		}

		uint32 GetStride() const
		{
			return Stride;
		}

		const aabbox3df& GetBBox() const
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

		void SetDefaultMaterial(TMaterialInstancePtr Material)
		{
			DefaultMaterial = Material;
		}

		TMaterialInstancePtr GetDefaultMaterial()
		{
			return DefaultMaterial;
		}
	protected:

	protected:
		E_PRIMITIVE_TYPE PrimitiveType;
		aabbox3df BBox;

		uint8* VsData;
		uint32 VsDataCount;

		E_INDEX_TYPE IndexType;
		uint8* PsData;
		uint32 PsDataCount;

		uint32 VsFormat;
		uint32 Stride;

		TMaterialInstancePtr DefaultMaterial;
	};

	///////////////////////////////////////////////////////////

	// TInstanceBuffer, hold instance data
	class TI_API TInstanceBuffer : public TResource
	{
	public:
		TInstanceBuffer();
		~TInstanceBuffer();

		static const int32 SemanticSize[EISI_TOTAL];
		static const int8* SemanticName[EISI_TOTAL];
		static const int32 SemanticIndex[EISI_TOTAL];

		static const uint32 InstanceFormat;
		static const uint32 InstanceStride;

	public:
		FInstanceBufferPtr InstanceResource;

		static int32 GetStrideFromFormat(uint32 Format);
		static TVector<E_INSTANCE_STREAM_INDEX> GetSteamsFromFormat(uint32 Format);
		void SetInstanceStreamData(
			uint32 InFormat, 
			const void* InInstanceData, int32 InInstanceCount
		);

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		int32 GetInstanceCount() const
		{
			return InstanceCount;
		}

		uint32 GetStride() const
		{
			return Stride;
		}

		const void* GetInstanceData() const
		{
			return InstanceData;
		}
	protected:

	protected:
		uint32 InsFormat;
		uint8* InstanceData;
		int32 InstanceCount;
		uint32 Stride;
	};
}
