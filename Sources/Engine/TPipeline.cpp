/*
	TiX Engine v2.0 Copyright (C) 2018~2021
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
		if (Desc.Shader->ShaderResource == nullptr)
		{
			Desc.Shader->InitRenderThreadResource();
		}
		PipelineResource = FRHI::Get()->CreatePipeline(Desc.Shader->ShaderResource);

		FPipelinePtr _PipelineResource = PipelineResource;
		TPipelinePtr PipelineDesc = this;
		ENQUEUE_RENDER_COMMAND(TPipelineUpdateResource)(
			[_PipelineResource, PipelineDesc]()
			{
				FRHI::Get()->UpdateHardwareResourcePL(_PipelineResource, PipelineDesc);
			});
	}

	void TPipeline::DestroyRenderThreadResource()
	{
		TI_ASSERT(PipelineResource != nullptr);

		FPipelinePtr _PipelineResource = PipelineResource;
		ENQUEUE_RENDER_COMMAND(TPipelineDestroyFPipeline)(
			[_PipelineResource]()
			{
				//_PipelineResource = nullptr;
			});
		PipelineResource = nullptr;
	}
    
    ////////////////////////////////////////////////////////////////////////
    
    TTilePipeline::TTilePipeline()
        : TResource(ERES_TILEPIPELINE)
        , RTCount(0)
        , SampleCount(1)
        , ThreadGroupSizeMatchesTileSize(1)
    {
    }
    
    TTilePipeline::~TTilePipeline()
    {
    }
}
