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

		EST_COUNT,
	};

	class FShader : public FRenderResource
	{
	public:
		FShader(const TString& ComputeShaderName);
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
			return ShaderNames.ShaderNames[0];
		}

		FShaderBindingPtr ShaderBinding;
	protected:

	protected:
		TShaderNames ShaderNames;
		E_SHADER_TYPE Type;

	};
}
