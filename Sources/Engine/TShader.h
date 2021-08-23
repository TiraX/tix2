/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_SHADER_STAGE
	{
		ESS_COMPUTE_SHADER = 0,
		ESS_SHADER_LIB = 0,
		ESS_VERTEX_SHADER = 0,
		ESS_PIXEL_SHADER,
		ESS_DOMAIN_SHADER,
		ESS_HULL_SHADER,
		ESS_GEOMETRY_SHADER,

		ESS_COUNT,
	};

	enum E_HITGROUP
	{
		HITGROUP_ANY_HIT,
		HITGROUP_CLOSEST_HIT,
		HITGROUP_INTERSECTION,

		HITGROUP_NUM
	};

	struct TShaderNames
	{
		TString ShaderNames[ESS_COUNT];

		TString GetSearchKey() const
		{
			return ShaderNames[ESS_VERTEX_SHADER] +
				ShaderNames[ESS_PIXEL_SHADER] +
				ShaderNames[ESS_DOMAIN_SHADER] +
				ShaderNames[ESS_HULL_SHADER] +
				ShaderNames[ESS_GEOMETRY_SHADER];
		}
	};

	class TShader : public TResource
	{
	public:
		// Used for Single Compute Shader or RTX Shaderlib
		TShader(const TString& InShaderName);

		// Used for Graphics pipeline with vs, gs, ps etc
		TShader(const TShaderNames& InNames);
		virtual ~TShader();
		
		void LoadShaderCode();
		const TStream& GetShaderCode(E_SHADER_STAGE InStage)
		{
			return ShaderCodes[InStage];
		}
		const TStream& GetComputeShaderCode()
		{
			return ShaderCodes[ESS_COMPUTE_SHADER];
		}
		const TStream& GetRtxShaderLibCode()
		{
			return ShaderCodes[ESS_SHADER_LIB];
		}

		void AddEntry(const TString& EntryName)
		{
			EntryNames.push_back(EntryName);
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

		void ReleaseShaderCode();

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		FShaderPtr ShaderResource;

	protected:
		TShaderNames Names;
		TStream ShaderCodes[ESS_COUNT];
		TVector<TString> EntryNames;
		TString HitGroupShaders[HITGROUP_NUM];
	};
}
