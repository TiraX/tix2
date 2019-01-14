/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FShader : public FRenderResource
	{
	public:
		FShader(const TShaderNames& InNames);
		virtual ~FShader();

		const TString& GetShaderName(E_SHADER_STAGE Stage) const
		{
			return ShaderNames.ShaderNames[Stage];
		}

	protected:

	protected:
		const TShaderNames& ShaderNames;
	};
}
