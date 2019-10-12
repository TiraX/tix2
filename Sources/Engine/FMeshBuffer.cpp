/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	FMeshBuffer::FMeshBuffer()
		: FRenderResource(RRT_VERTEX_BUFFER)
		, PrimitiveType(EPT_TRIANGLELIST)
		, Usage(TMeshBuffer::USAGE_DEFAULT)
		, VsDataCount(0)
		, IndexType(EIT_16BIT)
		, PsDataCount(0)
		, VsFormat(0)
		, Stride(0)
	{
	}

	FMeshBuffer::FMeshBuffer(
		E_PRIMITIVE_TYPE InPrimType,
		uint32 InVSFormat, 
		uint32 InVertexCount, 
		E_INDEX_TYPE InIndexType,
		uint32 InIndexCount)
		: FRenderResource(RRT_VERTEX_BUFFER)
		, PrimitiveType(InPrimType)
		, Usage(TMeshBuffer::USAGE_DEFAULT)
		, VsDataCount(InVertexCount)
		, IndexType(InIndexType)
		, PsDataCount(InIndexCount)
		, VsFormat(InVSFormat)
		, Stride(0)
	{
		Stride = TMeshBuffer::GetStrideFromFormat(InVSFormat);
	}

	FMeshBuffer::~FMeshBuffer()
	{
	}

	void FMeshBuffer::SetFromTMeshBuffer(TMeshBufferPtr InMeshBuffer)
	{
		PrimitiveType = InMeshBuffer->GetPrimitiveType();
		Usage = InMeshBuffer->GetUsage();
		VsDataCount = InMeshBuffer->GetVerticesCount();
		IndexType = InMeshBuffer->GetIndexType();
		PsDataCount = InMeshBuffer->GetIndicesCount();
		VsFormat = InMeshBuffer->GetVSFormat();
		Stride = InMeshBuffer->GetStride();
	}

	///////////////////////////////////////////////////////////
	FInstanceBuffer::FInstanceBuffer()
		: FRenderResource(RRT_INSTANCE_BUFFER)
		, InstanceCount(0)
		, Stride(0)
	{
	}

	FInstanceBuffer::~FInstanceBuffer()
	{
	}

	void FInstanceBuffer::SetFromTInstanceBuffer(TInstanceBufferPtr InInstanceData)
	{
		InstanceCount = InInstanceData->GetInstanceCount();
		Stride = InInstanceData->GetStride();
	}
}