/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FComputeTask.h"

namespace tix
{
	FComputeTask::FComputeTask(const TString& ComputeShaderName)
		: ShaderName(ComputeShaderName)
	{
	}

	FComputeTask::~FComputeTask()
	{
	}
}