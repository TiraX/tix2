/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TMeshBuffer.h"

namespace tix
{
	const int32 TMeshBuffer::SemanticSize[ESSI_TOTAL] =
	{
		12,	// ESSI_POSITION,
		4,	// ESSI_NORMAL,
		4,	// ESSI_COLOR,
		8,	// ESSI_TEXCOORD0,	// TI_TODO("May use half float texcoord in future");
		8,	// ESSI_TEXCOORD1,	// TI_TODO("May use half float texcoord in future");
		4,	// ESSI_TANGENT,	// TI_TODO("May use packed tangent in future");
		4,	// ESSI_BLENDINDEX,
		16,	// ESSI_BLENDWEIGHT,// TI_TODO("May use half float blend weight in future");
	};

	const int8* TMeshBuffer::SemanticName[ESSI_TOTAL] =
	{
		"POSITION",		// ESSI_POSITION,
		"NORMAL",		// ESSI_NORMAL,		// TI_TODO("May use packed normal in future");
		"COLOR",		// ESSI_COLOR,
		"TEXCOORD",		// ESSI_TEXCOORD0,	// TI_TODO("May use half float texcoord in future");
		"TEXCOORD",		// ESSI_TEXCOORD1,	// TI_TODO("May use half float texcoord in future");
		"TANGENT",		// ESSI_TANGENT,	// TI_TODO("May use packed tangent in future");
		"BLENDINDEX",	// ESSI_BLENDINDEX,
		"BLENDWEIGHT",	// ESSI_BLENDWEIGHT,// TI_TODO("May use half float blend weight in future");
	};

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
		TI_TODO("Change uv0 & uv1 stream to 4 component input.");
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
				RHI->UpdateHardwareResource(MeshBuffer, InMeshData);
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
}