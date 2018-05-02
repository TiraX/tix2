/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	FMeshBuffer::FMeshBuffer(E_MB_TYPES type)
		: Type(type)
		, PrimitiveType(EPT_TRIANGLELIST)
		, VsData(nullptr)
		, VsDataCount(0)
		, IndexType(EIT_16BIT)
		, PsData(nullptr)
		, PsDataCount(0)
		, VsFormat(0)
		, MeshFlag(0)
	{
	}

	FMeshBuffer::~FMeshBuffer()
	{
	}

	void FMeshBuffer::SetVertexStreamData(
		uint32 InFormat,
		void* InVertexData, int32 InVertexCount,
		E_INDEX_TYPE InIndexType, void* InIndexData, int32 InIndexCount)
	{
		VsFormat = InFormat;
		VsDataCount = InVertexCount;

		IndexType = InIndexType;
		PsDataCount = InIndexCount;

		VsData = (uint8*)InVertexData;
		PsData = (uint8*)InIndexData;

		CreateHardwareBuffer();
	}
}