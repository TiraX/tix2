/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
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
		TShader(const TShaderNames& InNames);
		virtual ~TShader();

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		FShaderPtr ShaderResource;

	protected:
		TShaderNames Names;
	};
}
