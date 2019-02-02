/*
	TiX Engine v2.0 Copyright (C) 2018~2019
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

	void TPipeline::SetShader(TShaderPtr Shader)
	{
		Desc.Shader = Shader;
	}
	
	void TPipeline::InitRenderThreadResource()
	{
		TI_ASSERT(PipelineResource == nullptr);
		PipelineResource = FRHI::Get()->CreatePipeline(Desc.Shader->ShaderResource);

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TPipelineUpdateResource,
			FPipelinePtr, Pipeline_RT, PipelineResource,
			TPipelinePtr, PipeDesc, this,
			{
				FRHI::Get()->UpdateHardwareResource(Pipeline_RT, PipeDesc);
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
