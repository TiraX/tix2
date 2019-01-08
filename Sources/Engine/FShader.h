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
		FShader(const TString& InShaderName);
		virtual ~FShader();

		const TString& GetShaderName() const
		{
			return ShaderName;
		}

	protected:

	protected:
		TString ShaderName;
	};
}
