/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FUniformBuffer.h"

namespace tix
{
	FUniformBuffer::FUniformBuffer(uint32 InStructureSizeInBytes, uint32 InElements)
		: StructureSizeInBytes(InStructureSizeInBytes)
		, Elements(InElements)
	{
	}

	FUniformBuffer::~FUniformBuffer()
	{
	}
}