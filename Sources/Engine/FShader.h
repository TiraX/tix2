/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_SHADER_TYPE
	{
		EST_RENDER,
		EST_COMPUTE,
		EST_SHADERLIB,

		EST_COUNT,
	};

	class FShader : public FRenderResource
	{
	public:
		FShader(const TString& ShaderName, E_SHADER_TYPE InType);
		FShader(const TShaderNames& RenderShaderNames);
		virtual ~FShader();

		E_SHADER_TYPE GetShaderType() const
		{
			return Type;
		}

		const TString& GetShaderName(E_SHADER_STAGE Stage) const
		{
			return ShaderNames.ShaderNames[Stage];
		}

		const TString& GetComputeShaderName() const
		{
			return ShaderNames.ShaderNames[ESS_COMPUTE_SHADER];
		}

		const TString& GetRtxShaderLibName() const
		{
			return ShaderNames.ShaderNames[ESS_SHADER_LIB];
		}
		const TVector<TString>& GetEntryNames() const
		{
			return EntryNames;
		}

		void SetHitGroupShader(E_HITGROUP HitGroup, const TString& InShaderName)
		{
			HitGroupShaders[HitGroup] = InShaderName;
		}
		const TString& GetHitGroupShader(E_HITGROUP HitGroup) const
		{
			return HitGroupShaders[HitGroup];
		}

		FShaderBindingPtr ShaderBinding;
	protected:

	protected:
		TShaderNames ShaderNames;
		E_SHADER_TYPE Type;
		TVector<TString> EntryNames;
		TString HitGroupShaders[HITGROUP_NUM];
	};
}
