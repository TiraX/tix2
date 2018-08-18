/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	TMaterial::TMaterial()
		: TResource(ERES_MATERIAL)
	{}

	TMaterial::~TMaterial()
	{
	}

	void TMaterial::InitRenderThreadResource()
	{
		TI_ASSERT(0);
	}

	void TMaterial::DestroyRenderThreadResource()
	{
		TI_ASSERT(0);
	}
}
