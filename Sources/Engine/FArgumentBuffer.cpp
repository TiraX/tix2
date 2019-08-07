/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FArgumentBuffer.h"

namespace tix
{
	FArgumentBuffer::FArgumentBuffer(int32 ReservedTextures)
		: FRenderResource(RRT_ARGUMENT_BUFFER)
	{
		ArgumentTextures.resize(ReservedTextures);
	}

	FArgumentBuffer::~FArgumentBuffer()
	{
	}

	void FArgumentBuffer::SetDataBuffer(const void * InData, int32 DataLength)
	{
		ArgumentDataBuffer.Reset();
		ArgumentDataBuffer.Put(InData, DataLength);
	}

	void FArgumentBuffer::SetTexture(int32 Index, FTexturePtr InTexture)
	{
		TI_ASSERT(Index < 16);
		if (ArgumentTextures.size() <= Index)
		{
			ArgumentTextures.resize(Index + 1);
		}
		ArgumentTextures[Index] = InTexture;
	}
}