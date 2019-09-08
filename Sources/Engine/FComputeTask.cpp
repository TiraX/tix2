/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FComputeTask.h"

namespace tix
{
	FComputeTask::FComputeTask(const TString& ComputeShaderName, uint32 InFlags)
		: ShaderName(ComputeShaderName)
        , Flags(InFlags)
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

		TShaderPtr ShaderCode = ti_new TShader(ShaderName);
		ShaderCode->LoadShaderCode();
		FRHI::Get()->UpdateHardwareResourceShader(ComputeShader, ShaderCode);

        if (HasFlag(COMPUTE_TILE))
        {
            // For metal : Create tile pipeline state
            TI_ASSERT(TilePLDesc->GetRTCount() > 0);
            FRHI::Get()->UpdateHardwareResourceTilePL(ComputePipeline, TilePLDesc);
        }
        else
        {
            TPipelinePtr PipelineDesc = nullptr;
            FRHI::Get()->UpdateHardwareResourcePL(ComputePipeline, PipelineDesc);
        }
	}
}
