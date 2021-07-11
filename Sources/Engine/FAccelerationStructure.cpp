/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FAccelerationStructure.h"

namespace tix
{
	FAccelerationStructure::FAccelerationStructure()
		: FRenderResource(RRT_ACCELERATION_STRUCTURE)
		, IsDirty(true)
	{
	}

	FAccelerationStructure::~FAccelerationStructure()
	{
	}

	/////////////////////////////////////////////////////////////
	FBottomLevelAccelerationStructure::FBottomLevelAccelerationStructure()
	{
	}

	FBottomLevelAccelerationStructure::~FBottomLevelAccelerationStructure()
	{
	}

	/////////////////////////////////////////////////////////////
	FTopLevelAccelerationStructure::FTopLevelAccelerationStructure()
	{
	}

	FTopLevelAccelerationStructure::~FTopLevelAccelerationStructure()
	{
	}
}
