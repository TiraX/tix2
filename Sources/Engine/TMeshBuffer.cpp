/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TMeshBuffer.h"
#include "TMeshBufferSemantic.h"

namespace tix
{
	TMeshBuffer::TMeshBuffer()
		: TResource(ERES_MESH)
		, PrimitiveType(EPT_TRIANGLELIST)
		, VsData(nullptr)
		, VsDataCount(0)
		, IndexType(EIT_16BIT)
		, PsData(nullptr)
		, PsDataCount(0)
		, VsFormat(0)
		, Stride(0)
		, MeshFlag(0)
	{
	}

	TMeshBuffer::~TMeshBuffer()
	{
		SAFE_DELETE(VsData);
		SAFE_DELETE(PsData);
	}

	int32 TMeshBuffer::GetStrideFromFormat(uint32 Format)
	{
		// Calculate stride
		int32 Stride = 0;
		for (uint32 seg = 1, i = 0; seg <= EVSSEG_TOTAL; seg <<= 1, ++i)
		{
			if ((Format & seg) != 0)
			{
				Stride += SemanticSize[i];
			}
		}
		return Stride;
	}

	TVector<E_MESH_STREAM_INDEX> TMeshBuffer::GetSteamsFromFormat(uint32 Format)
	{
		TVector<E_MESH_STREAM_INDEX> Streams;
		for (uint32 seg = 1, i = 0; seg <= EVSSEG_TOTAL; seg <<= 1, ++i)
		{
			if ((Format & seg) != 0)
			{
				Streams.push_back((E_MESH_STREAM_INDEX)i);
			}
		}
		return Streams;
	}

	void TMeshBuffer::InitRenderThreadResource()
	{
		TI_ASSERT(MeshBufferResource == nullptr);
		MeshBufferResource = FRHI::Get()->CreateMeshBuffer();

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TMeshBufferUpdateFMeshBuffer,
			FMeshBufferPtr, MeshBuffer, MeshBufferResource,
			TMeshBufferPtr, InMeshData, this,
			{
				MeshBuffer->SetFromTMeshBuffer(InMeshData);
				FRHI::Get()->UpdateHardwareResource(MeshBuffer, InMeshData);
			});
	}

	void TMeshBuffer::DestroyRenderThreadResource()
	{
		TI_ASSERT(MeshBufferResource != nullptr);

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(TMeshBufferDestroyFMeshBuffer,
			FMeshBufferPtr, MeshBuffer, MeshBufferResource,
			{
				MeshBuffer = nullptr;
			});
		MeshBufferResource = nullptr;
	}

	void TMeshBuffer::SetVertexStreamData(
		uint32 InFormat,
		const void* InVertexData, int32 InVertexCount,
		E_INDEX_TYPE InIndexType,
		const void* InIndexData, int32 InIndexCount)
	{
		VsFormat = InFormat;
		VsDataCount = InVertexCount;

		IndexType = InIndexType;
		PsDataCount = InIndexCount;

		Stride = GetStrideFromFormat(VsFormat);

		const uint32 VsSize = InVertexCount * Stride;
		const uint32 VsBufferSize = ti_align(VsSize, 16);
		TI_ASSERT(VsData == nullptr);
		VsData = ti_new uint8[VsBufferSize];
		memcpy(VsData, InVertexData, VsSize);

		const uint32 PsSize = InIndexCount * (InIndexType == EIT_16BIT ? sizeof(uint16) : sizeof(uint32));
		const uint32 PsBufferSize = ti_align(PsSize, 16);
		TI_ASSERT(PsData == nullptr);
		PsData = ti_new uint8[PsBufferSize];
		memcpy(PsData, InIndexData, PsSize);
	}

	///////////////////////////////////////////////////////////

	TInstanceBuffer::TInstanceBuffer()
		: TResource(ERES_INSTANCE)
		, InstanceData(nullptr)
		, InstanceCount(0)
		, Stride(0)
	{
	}

	TInstanceBuffer::~TInstanceBuffer()
	{
		SAFE_DELETE(InstanceData);
	}

	void TInstanceBuffer::SetInstanceStreamData(const void* InInstanceData, int32 InInstanceCount, int32 InStride)
	{
		InstanceCount = InInstanceCount;
		Stride = InStride;

		const int32 BufferSize = InstanceCount * Stride;
		TI_ASSERT(InstanceData == nullptr);
		InstanceData = ti_new uint8[BufferSize];
		memcpy(InstanceData, InInstanceData, BufferSize);
	}

	void TInstanceBuffer::InitRenderThreadResource()
	{
		TI_ASSERT(InstanceResource == nullptr);
		InstanceResource = FRHI::Get()->CreateInstanceBuffer();

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TInstanceBufferUpdateFInstanceBuffer,
			FInstanceBufferPtr, InstanceBuffer, InstanceResource,
			TInstanceBufferPtr, InInstanceData, this,
			{
				InstanceBuffer->SetFromTInstanceBuffer(InInstanceData);
				FRHI::Get()->UpdateHardwareResource(InstanceBuffer, InInstanceData);
			});
	}

	void TInstanceBuffer::DestroyRenderThreadResource()
	{
		TI_ASSERT(InstanceResource != nullptr);

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(TInstanceBufferDestroyFInstanceBuffer,
			FInstanceBufferPtr, InstanceBuffer, InstanceResource,
			{
				InstanceBuffer = nullptr;
			});
		InstanceResource = nullptr;
	}
}