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
		TI_TODO("0. Continue with pipeline.");
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
		TI_TODO("Try to reference DESC as InstrusivePtr directly. Do not pass TPipelinePtr, pass TPipelineDescPtr instead");
		TI_ASSERT(PipelineResource == nullptr);
		PipelineResource = FRHI::Get()->CreatePipeline();

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TPipelineUpdateResource,
			FPipelinePtr, Pipeline_RT, PipelineResource,
			TPipelinePtr, PipeDesc, this,
			{
				RHI->UpdateHardwareBuffer(Pipeline_RT, PipeDesc);
			});
	}

	void TPipeline::DestroyRenderThreadResource()
	{
		TI_TODO("2. Destroy pipeline render thread resource");
	}
}
