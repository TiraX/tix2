/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FUniformBuffer.h"

namespace tix
{
	FUniformBuffer::FUniformBuffer(E_RESOURCE_FAMILY InFamily)
		: FRenderResource(InFamily)
	{
	}

	FUniformBuffer::~FUniformBuffer()
	{
	}
}