/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TShader.h"

namespace tix
{
	TShader::TShader(const TString& InShaderName)
		: TResource(ERES_SHADER)
	{
		Names.ShaderNames[0] = InShaderName;
	}

	TShader::TShader(const TShaderNames& InNames)
		: TResource(ERES_SHADER)
	{
		for (int32 s = 0; s < ESS_COUNT; ++s)
		{
			Names.ShaderNames[s] = InNames.ShaderNames[s];
		}
	}

	TShader::~TShader()
	{
	}

	void TShader::LoadShaderCode()
	{
		for (int32 s = 0; s < ESS_COUNT; ++s)
		{
			TString ShaderName = Names.ShaderNames[s];
			if (!ShaderName.empty())
			{
#if defined (COMPILE_WITH_RHI_DX12)
				if (ShaderName.rfind(".cso") == TString::npos)
					ShaderName += ".cso";
                // Load shader code
                TFile File;
                if (File.Open(ShaderName, EFA_READ))
                {
                    ShaderCodes[s].Put(File);
                    File.Close();
                }
                else
                {
                    _LOG(Fatal, "Failed to load shader code [%s].\n", ShaderName.c_str());
                }
#elif defined (COMPILE_WITH_RHI_METAL)
                // Metal need shader name only
#else
				TI_ASSERT(0);
#endif
			}
		}
	}

	void TShader::InitRenderThreadResource()
	{
		TI_ASSERT(ShaderResource == nullptr);
		ShaderResource = FRHI::Get()->CreateShader(Names);

		FShaderPtr Shader_RT = ShaderResource;
		TShaderPtr ShaderSource = this;
		ENQUEUE_RENDER_COMMAND(TShaderUpdateResource)(
			[Shader_RT, ShaderSource]()
			{
				// Add TShader -> Shader Codes herer.
				FRHI::Get()->UpdateHardwareResourceShader(Shader_RT, ShaderSource);
			});
	}

	void TShader::ReleaseShaderCode()
	{
		for (int32 s = 0; s < ESS_COUNT; ++s)
		{
			ShaderCodes[s].Destroy();
		}
	}

	void TShader::DestroyRenderThreadResource()
	{
		TI_ASSERT(ShaderResource != nullptr);

		FShaderPtr Shader_RT = ShaderResource;
		ENQUEUE_RENDER_COMMAND(TShaderDestroyFShader)(
			[Shader_RT]()
			{
				//Shader_RT = nullptr;
			});
		ShaderResource = nullptr;
	}
}
