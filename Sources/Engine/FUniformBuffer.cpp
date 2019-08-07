/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FUniformBuffer.h"

namespace tix
{
	FUniformBuffer::FUniformBuffer(uint32 InStructureSizeInBytes, uint32 InElements, uint32 InUBFlag)
		: FRenderResource(RRT_UNIFORM_BUFFER)
		, StructureSizeInBytes(InStructureSizeInBytes)
		, Elements(InElements)
		, Flag(InUBFlag)
	{
	}

	FUniformBuffer::~FUniformBuffer()
	{
	}
}