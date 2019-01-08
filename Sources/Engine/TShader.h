/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TShader : public TResource
	{
	public:
		TShader(const TString& InName);
		virtual ~TShader();

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		FShaderPtr ShaderResource;

	protected:
		TString Name;
	};
}
