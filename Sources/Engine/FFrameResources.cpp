/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHI.h"
#include "FFrameResources.h"

namespace tix
{
	FFrameResources::FFrameResources()
	{
		//Resources.reserve(DefaultReserveCount);
	}

	FFrameResources::~FFrameResources()
	{
		RemoveAllReferences();
	}

	void FFrameResources::RemoveAllReferences()
	{
		for (auto& R : Resources)
		{
			R = nullptr;
		}
		Resources.clear();
	}

	void FFrameResources::HoldReference(FRenderResourcePtr InResource)
	{
		Resources.push_back(InResource);
	}
}