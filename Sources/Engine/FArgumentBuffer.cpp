/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FArgumentBuffer.h"

namespace tix
{
	FArgumentBuffer::FArgumentBuffer(int32 ReservedSlots)
		: FRenderResource(RRT_ARGUMENT_BUFFER)
	{
		Arguments.resize(ReservedSlots);
	}

	FArgumentBuffer::~FArgumentBuffer()
	{
	}

	void FArgumentBuffer::SetBuffer(int32 Index, FUniformBufferPtr InUniform)
	{
		TI_ASSERT(Index < MaxResourcesInArgumentBuffer);
		if (Arguments.size() <= Index)
		{
			Arguments.resize(Index + 1);
		}
		Arguments[Index] = InUniform;
	}

	void FArgumentBuffer::SetTexture(int32 Index, FTexturePtr InTexture)
	{
		TI_ASSERT(Index < MaxResourcesInArgumentBuffer);
		if (Arguments.size() <= Index)
		{
			Arguments.resize(Index + 1);
		}
		Arguments[Index] = InTexture;
	}
}