/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TMeshBuffer.h"
#include "TMeshBufferSemantic.h"

namespace tix
{
	TMeshBuffer::TMeshBuffer()
		: TResource(ERES_MESH_BUFFER)
		, PrimitiveType(EPT_TRIANGLELIST)
		, VsData(nullptr)
		, VsDataCount(0)
		, IndexType(EIT_16BIT)
		, PsData(nullptr)
		, PsDataCount(0)
		, VsFormat(0)
		, Stride(0)
	{
	}

	TMeshBuffer::~TMeshBuffer()
	{
		SAFE_DELETE(VsData);
		SAFE_DELETE(PsData);
	}

	uint32 TMeshBuffer::GetStrideFromFormat(uint32 Format)
	{
		// Calculate stride
		uint32 Stride = 0;
		for (uint32 seg = 1, i = 0; seg <= EVSSEG_TOTAL; seg <<= 1, ++i)
		{
			if ((Format & seg) != 0)
			{
				Stride += TMeshBuffer::SemanticSize[i];
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
		if (ClusterData.size() > 0)
		{
			TI_ASSERT(MeshClusterDataResource == nullptr);
			MeshClusterDataResource = FRHI::Get()->CreateUniformBuffer(sizeof(TMeshClusterData), (uint32)ClusterData.size(), UB_FLAG_INTERMEDIATE);
		}

		ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(TMeshBufferUpdateFMeshBuffer,
			FMeshBufferPtr, MeshBuffer, MeshBufferResource,
			FUniformBufferPtr, ClusterDataBuffer, MeshClusterDataResource,
			TMeshBufferPtr, InMeshData, this,
			{
				MeshBuffer->SetFromTMeshBuffer(InMeshData);
				FRHI::Get()->UpdateHardwareResourceMesh(MeshBuffer, InMeshData);
				if (ClusterDataBuffer != nullptr)
				{
					FRHI::Get()->UpdateHardwareResourceUB(ClusterDataBuffer, InMeshData->GetClusterData().data());
				}
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
		const void* InVertexData, uint32 InVertexCount,
		E_INDEX_TYPE InIndexType,
		const void* InIndexData, uint32 InIndexCount)
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

	void TMeshBuffer::SetClusterData(const void* InClusterData, uint32 InClusterCount)
	{
		TI_ASSERT(InClusterCount > 0);
		ClusterData.resize(InClusterCount);

		const TMeshClusterDef* InData = (const TMeshClusterDef*)InClusterData;
		for (uint32 c = 0 ; c < InClusterCount ; ++ c)
		{
			ClusterData[c].MinEdge = InData[c].BBox.MinEdge;
			ClusterData[c].MaxEdge = InData[c].BBox.MaxEdge;
			ClusterData[c].Cone = InData[c].Cone;
		}
	}

	///////////////////////////////////////////////////////////

	const uint32 TInstanceBuffer::InstanceFormat = EINSSEG_TRANSITION | EINSSEG_ROT_SCALE_MAT0 | EINSSEG_ROT_SCALE_MAT1 | EINSSEG_ROT_SCALE_MAT2;
	const uint32 TInstanceBuffer::InstanceStride =
		TInstanceBuffer::SemanticSize[EISI_TRANSITION] +
		TInstanceBuffer::SemanticSize[EISI_ROT_SCALE_MAT0] +
		TInstanceBuffer::SemanticSize[EISI_ROT_SCALE_MAT1] +
		TInstanceBuffer::SemanticSize[EISI_ROT_SCALE_MAT2];

	TInstanceBuffer::TInstanceBuffer()
		: TResource(ERES_INSTANCE)
		, InsFormat(0)
		, InstanceData(nullptr)
		, InstanceCount(0)
		, Stride(0)
	{
	}

	TInstanceBuffer::~TInstanceBuffer()
	{
		SAFE_DELETE(InstanceData);
	}

	int32 TInstanceBuffer::GetStrideFromFormat(uint32 Format)
	{
		// Calculate stride
		int32 Stride = 0;
		for (uint32 seg = 1, i = 0; seg <= EINSSEG_TOTAL; seg <<= 1, ++i)
		{
			if ((Format & seg) != 0)
			{
				Stride += TInstanceBuffer::SemanticSize[i];
			}
		}
		return Stride;
	}

	TVector<E_INSTANCE_STREAM_INDEX> TInstanceBuffer::GetSteamsFromFormat(uint32 Format)
	{
		TVector<E_INSTANCE_STREAM_INDEX> Streams;
		for (uint32 seg = 1, i = 0; seg <= EINSSEG_TOTAL; seg <<= 1, ++i)
		{
			if ((Format & seg) != 0)
			{
				Streams.push_back((E_INSTANCE_STREAM_INDEX)i);
			}
		}
		return Streams;
	}

	void TInstanceBuffer::SetInstanceStreamData(
		uint32 InFormat, 
		const void* InInstanceData, int32 InInstanceCount
	)
	{
		InsFormat = InFormat;
		InstanceCount = InInstanceCount;
		Stride = GetStrideFromFormat(InsFormat);

		const int32 BufferSize = InstanceCount * Stride;
		TI_ASSERT(InstanceData == nullptr);
		InstanceData = ti_new uint8[BufferSize];
		memcpy(InstanceData, InInstanceData, BufferSize);
	}

	void TInstanceBuffer::InitRenderThreadResource()
	{
		TI_ASSERT(InstanceResource == nullptr);
		InstanceResource = FRHI::Get()->CreateInstanceBuffer();
		// Set Instance Resource Usage to USAGE_COPY_SOURCE, 
		// as GPU Driven pipeline need to copy these instance buffer into a merged instance buffer
		TI_TODO("Add gpu driven CVAR to check here.");
		//InstanceResource->SetUsage(FRenderResource::USAGE_COPY_SOURCE);

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TInstanceBufferUpdateFInstanceBuffer,
			FInstanceBufferPtr, InstanceBuffer, InstanceResource,
			TInstanceBufferPtr, InInstanceData, this,
			{
				InstanceBuffer->SetFromTInstanceBuffer(InInstanceData);
				FRHI::Get()->UpdateHardwareResourceIB(InstanceBuffer, InInstanceData);
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