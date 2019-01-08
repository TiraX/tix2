/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TShader.h"

namespace tix
{
	TShader::TShader(const TString& InName)
		: TResource(ERES_SHADER)
		, Name(InName)
	{
	}

	TShader::~TShader()
	{
	}

	void TShader::InitRenderThreadResource()
	{
		TI_ASSERT(ShaderResource == nullptr);
		ShaderResource = FRHI::Get()->CreateShader(Name);

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(TShaderUpdateResource,
			FShaderPtr, Shader_RT, ShaderResource,
			{
				FRHI::Get()->UpdateHardwareResource(Shader_RT);
			});
	}

	void TShader::DestroyRenderThreadResource()
	{
		TI_ASSERT(ShaderResource != nullptr);

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(TShaderDestroyFShader,
			FShaderPtr, Shader_RT, ShaderResource,
			{
				Shader_RT = nullptr;
			});
		ShaderResource = nullptr;
	}
}
