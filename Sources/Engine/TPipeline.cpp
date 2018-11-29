/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TPipeline.h"

namespace tix
{
	TPipeline::TPipeline()
		: TResource(ERES_PIPELINE)
	{
	}

	TPipeline::~TPipeline()
	{
	}

	void TPipeline::SetShader(E_SHADER_STAGE ShaderStage, const TString& ShaderName, const int8* InShaderCode, int32 CodeLength)
	{
		Desc.ShaderName[ShaderStage] = ShaderName;
		ShaderCode[ShaderStage].Put(InShaderCode, CodeLength);
	}
	
	void TPipeline::InitRenderThreadResource()
	{
		TI_ASSERT(PipelineResource == nullptr);
		PipelineResource = FRHI::Get()->CreatePipeline();
		TI_ASSERT(ShaderBinding->ShaderBindingResource != nullptr);
		PipelineResource->SetShaderBinding(ShaderBinding->ShaderBindingResource);

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TPipelineUpdateResource,
			FPipelinePtr, Pipeline_RT, PipelineResource,
			TPipelinePtr, PipeDesc, this,
			{
				RHI->UpdateHardwareResource(Pipeline_RT, PipeDesc);
			});
	}

	void TPipeline::DestroyRenderThreadResource()
	{
		TI_ASSERT(PipelineResource != nullptr);

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(TPipelineDestroyFPipeline,
			FPipelinePtr, Pipeline_RT, PipelineResource,
			{
				Pipeline_RT = nullptr;
			});
		PipelineResource = nullptr;
	}
}
