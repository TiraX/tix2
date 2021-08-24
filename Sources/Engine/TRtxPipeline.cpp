/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TRtxPipeline.h"

namespace tix
{
	TRtxPipeline::TRtxPipeline()
		: TResource(ERES_RTX_PIPELINE)
	{
	}

	TRtxPipeline::~TRtxPipeline()
	{
	}

	void TRtxPipeline::SetShaderLib(TShaderPtr InShaderLib)
	{
		Desc.ShaderLib = InShaderLib;
	}

	void TRtxPipeline::AddExportName(const TString& InName)
	{
		Desc.ExportNames.push_back(InName);
	}

	void TRtxPipeline::SetHitGroup(E_HITGROUP HitGroup, const TString& InName)
	{
		Desc.HitGroup[HitGroup] = InName;
	}
	
	void TRtxPipeline::InitRenderThreadResource()
	{
		TI_ASSERT(PipelineResource == nullptr);
		if (Desc.ShaderLib->ShaderResource == nullptr)
		{
			Desc.ShaderLib->InitRenderThreadResource();
		}
		PipelineResource = FRHI::Get()->CreateRtxPipeline(Desc.ShaderLib->ShaderResource);

		FRtxPipelinePtr _PipelineResource = PipelineResource;
		TRtxPipelinePtr PipelineDesc = this;
		ENQUEUE_RENDER_COMMAND(TRtxPipelineUpdateResource)(
			[_PipelineResource, PipelineDesc]()
			{
				FRHI::Get()->UpdateHardwareResourceRtxPL(_PipelineResource, PipelineDesc);
			});
	}

	void TRtxPipeline::DestroyRenderThreadResource()
	{
		TI_ASSERT(PipelineResource != nullptr);

		FRtxPipelinePtr _PipelineResource = PipelineResource;
		ENQUEUE_RENDER_COMMAND(TRtxPipelineDestroyFPipeline)(
			[_PipelineResource]()
			{
				//_PipelineResource = nullptr;
			});
		PipelineResource = nullptr;
	}
}
