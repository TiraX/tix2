/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FComputeTaskDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FComputeTaskDx12::FComputeTaskDx12(const TString& ComputeShaderName)
		: FComputeTask(ComputeShaderName)
	{
	}

	FComputeTaskDx12::~FComputeTaskDx12()
	{
	}
}

#endif	// COMPILE_WITH_RHI_DX12