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
		ComputeShader = FRHI::Get()->CreateComputeShader(ComputeShaderName);
		ComputePipeline = FRHI::Get()->CreatePipeline(ComputeShader);
	}

	FComputeTask::~FComputeTask()
	{
	}

	void FComputeTask::Finalize()
	{
		if (IsRenderThread())
		{
			FinalizeInRenderThread();
		}
		else
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(FComputeTaskFinalize,
				FComputeTaskPtr, ComputeTask, this,
				{
					ComputeTask->FinalizeInRenderThread();
				});
		}
	}

	void FComputeTask::FinalizeInRenderThread()
	{
		TI_ASSERT(IsRenderThread());

		TI_ASSERT(0);
		//FRHI::Get()->UpdateHardwareResource(ComputeShader);

		TPipelinePtr PipelineDesc = nullptr;
		FRHI::Get()->UpdateHardwareResource(ComputePipeline, PipelineDesc);
	}
}