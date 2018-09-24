/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

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
	FMeshBuffer::FMeshBuffer(E_RESOURCE_FAMILY InFamily)
		: FRenderResource(InFamily)
		, PrimitiveType(EPT_TRIANGLELIST)
		, VsDataCount(0)
		, IndexType(EIT_16BIT)
		, PsDataCount(0)
		, VsFormat(0)
		, Stride(0)
		, MeshFlag(0)
	{
	}

	FMeshBuffer::~FMeshBuffer()
	{
	}

	void FMeshBuffer::SetFromTMeshBuffer(TMeshBufferPtr InMeshBuffer)
	{
		PrimitiveType = InMeshBuffer->GetPrimitiveType();
		Usage = InMeshBuffer->GetUsage();
		MeshFlag = InMeshBuffer->GetFlag();
		VsDataCount = InMeshBuffer->GetVerticesCount();
		IndexType = InMeshBuffer->GetIndexType();
		PsDataCount = InMeshBuffer->GetIndicesCount();
		VsFormat = InMeshBuffer->GetVSFormat();
		Stride = InMeshBuffer->GetStride();
	}
}