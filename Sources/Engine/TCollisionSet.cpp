/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TCollisionSet.h"

namespace tix
{
	TCollisionSet::TCollisionSet()
		: TResource(ERES_COLLISION)
	{
	}

	TCollisionSet::~TCollisionSet()
	{
	}

	void TCollisionSet::InitRenderThreadResource()
	{
	}

	void TCollisionSet::DestroyRenderThreadResource()
	{
	}
}