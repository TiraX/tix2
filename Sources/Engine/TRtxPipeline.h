/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TRtxPipelineDesc
	{
		uint32 Flags;
		TShaderPtr ShaderLib;

		TRtxPipelineDesc()
			: Flags(0)
		{
		}
	};

    class TRtxPipeline : public TResource
	{
	public:
		TRtxPipeline();
		virtual ~TRtxPipeline();

		void SetShaderLib(TShaderPtr InShaderLib);

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		FRtxPipelinePtr PipelineResource;

	protected:

	protected:
		TRtxPipelineDesc Desc;
	};
}
