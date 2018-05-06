/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FFrameResourcesDx12.h"

namespace tix
{
	FFrameResourcesDx12::FFrameResourcesDx12()
	{
		D3d12Resources.reserve(FFrameResources::DefaultReserveCount);
	}

	FFrameResourcesDx12::~FFrameResourcesDx12()
	{
	}

	void FFrameResourcesDx12::RemoveAllReferences()
	{
		FFrameResources::RemoveAllReferences();

		for (auto& R : D3d12Resources)
		{
			R = nullptr;
		}
		D3d12Resources.clear();
	}
}