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

		FRHI::Get()->UpdateHardwareResource(ComputeShader);

		TPipelinePtr PipelineDesc = nullptr;
		FRHI::Get()->UpdateHardwareResource(ComputePipeline, PipelineDesc);

		const int32 NumBindings = ComputeShader->ShaderBinding->GetNumBinding();
		Resources.resize(NumBindings);
	}

	void FComputeTask::SetConstantBuffer(int32 Index, FUniformBufferPtr Uniform)
	{
		Resources[Index] = Uniform;
	}

	void FComputeTask::SetParameter(int32 Index, FRenderResourceTablePtr ResourceTable)
	{
		Resources[Index] = ResourceTable;
	}

	void FComputeTask::Run(FRHI * RHI)
	{
		TI_TODO("Correct this test logic. ");
		RHI->SetPipeline(ComputePipeline);
		RHI->SetComputeConstantBuffer(0, ResourceCast<FUniformBuffer>(Resources[0]));
		RHI->SetComputeResourceTable(1, ResourceCast<FRenderResourceTable>(Resources[1]));
	}
}