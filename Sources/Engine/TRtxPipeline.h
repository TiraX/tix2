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

		TVector<TString> ExportNames;
		TString HitGroup[HITGROUP_NUM];

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
		void AddExportName(const TString& InName);
		void SetHitGroup(E_HITGROUP HitGroup, const TString& InName);

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		FRtxPipelinePtr PipelineResource;

	protected:

	protected:
		TRtxPipelineDesc Desc;
	};
}
