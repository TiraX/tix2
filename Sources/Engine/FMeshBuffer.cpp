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

	///////////////////////////////////////////////////////////
	FInstanceBuffer::FInstanceBuffer()
		: FRenderResource(RRT_INSTANCE_BUFFER)
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