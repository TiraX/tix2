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
		uint32 InIndexCount,
		const aabbox3df& InBBox
	)
		: FRenderResource(RRT_VERTEX_BUFFER)
		, PrimitiveType(InPrimType)
		, VsDataCount(InVertexCount)
		, IndexType(InIndexType)
		, PsDataCount(InIndexCount)
		, VsFormat(InVSFormat)
		, Stride(0)
		, BBox(InBBox)
	{
		Stride = TMeshBuffer::GetStrideFromFormat(InVSFormat);
	}

	FMeshBuffer::~FMeshBuffer()
	{
	}

	void FMeshBuffer::SetFromTMeshBuffer(TMeshBufferPtr InMeshBuffer)
	{
		PrimitiveType = InMeshBuffer->GetPrimitiveType();
		VsDataCount = InMeshBuffer->GetVerticesCount();
		IndexType = InMeshBuffer->GetIndexType();
		PsDataCount = InMeshBuffer->GetIndicesCount();
		VsFormat = InMeshBuffer->GetVSFormat();
		Stride = InMeshBuffer->GetStride();
		BBox = InMeshBuffer->GetBBox();
	}

	///////////////////////////////////////////////////////////
	FInstanceBuffer::FInstanceBuffer()
		: FRenderResource(RRT_INSTANCE_BUFFER)
		, InstanceCount(0)
		, Stride(0)
	{
	}

	FInstanceBuffer::FInstanceBuffer(uint32 TotalInstancesCount, uint32 InstanceStride)
		: FRenderResource(RRT_INSTANCE_BUFFER)
		, InstanceCount(TotalInstancesCount)
		, Stride(InstanceStride)
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