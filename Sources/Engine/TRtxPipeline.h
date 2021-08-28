/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_HITGROUP
	{
		HITGROUP_ANY_HIT,
		HITGROUP_CLOSEST_HIT,
		HITGROUP_INTERSECTION,

		HITGROUP_NUM
	};

	struct TRtxPipelineDesc
	{
		uint32 Flags;
		TShaderPtr ShaderLib;

		TVector<TString> ExportNames;
		TString HitGroupName;
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
		void SetHitGroupName(const TString& InName);
		void SetHitGroup(E_HITGROUP HitGroup, const TString& InName);

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		const TRtxPipelineDesc& GetDesc() const
		{
			return Desc;
		}

		FRtxPipelinePtr PipelineResource;

	protected:

	protected:
		TRtxPipelineDesc Desc;
	};
}
