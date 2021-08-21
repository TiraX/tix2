/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
    class TRtxPipeline : public TResource
	{
	public:
		TRtxPipeline();
		virtual ~TRtxPipeline();

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		FRtxPipelinePtr PipelineResource;

	protected:

	protected:
	};
}
