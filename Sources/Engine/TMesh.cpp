/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TMesh.h"

namespace tix
{
	static const int32 SematicSize[ESSI_TOTAL] =
	{
		12,	// ESSI_POSITION,
		12,	// ESSI_NORMAL,		// TI_TODO("May use packed normal in future");
		4,	// ESSI_COLOR,
		8,	// ESSI_TEXCOORD0,	// TI_TODO("May use half float texcoord in future");
		8,	// ESSI_TEXCOORD1,	// TI_TODO("May use half float texcoord in future");
		12,	// ESSI_TANGENT,	// TI_TODO("May use packed tangent in future");
		4,	// ESSI_BLENDINDEX,
		16,	// ESSI_BLENDWEIGHT,// TI_TODO("May use half float blend weight in future");
	};
	TMeshBuffer::TMeshBuffer()
		: PrimitiveType(EPT_TRIANGLELIST)
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

		// Calculate stride
		Stride = 0;
		for (UINT seg = 1, i = 0; seg <= EVSSEG_TOTAL; seg <<= 1, ++i)
		{
			if (VsFormat & seg)
			{
				Stride += SematicSize[i];
			}
		}

		const uint32 VsSize = InVertexCount * Stride;
		const uint32 VsBufferSize = ti_align(VsSize, 16);
		VsData = ti_new uint8[VsBufferSize];
		memcpy(VsData, InVertexData, VsSize);

		const uint32 PsSize = InIndexCount * (InIndexType == EIT_16BIT ? sizeof(uint16) : sizeof(uint32));
		const uint32 PsBufferSize = ti_align(PsSize, 16);
		PsData = ti_new uint8[PsBufferSize];
		memcpy(PsData, InIndexData, PsSize);
	}
}