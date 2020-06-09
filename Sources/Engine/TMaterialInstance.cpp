/*
TiX Engine v2.0 Copyright (C) 2018~2021
By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	TMaterialInstance::TMaterialInstance()
		: TResource(ERES_MATERIAL_INSTANCE)
	{}

	TMaterialInstance::~TMaterialInstance()
	{
	}

	void TMaterialInstance::InitRenderThreadResource()
	{
		if (ParamValueBuffer != nullptr || ParamTextureNames.size() > 0)
		{
			TI_ASSERT(ArgumentBuffer == nullptr);
			TI_ASSERT(LinkedMaterial->GetDesc().Shader != nullptr);
			int32 NumBuffers = ParamValueBuffer != nullptr ? 1 : 0;
			int32 NumTextures = (int32)ParamTextureNames.size();
			int32 NumArguments = NumBuffers + NumTextures;
			ArgumentBuffer = FRHI::Get()->CreateArgumentBuffer(NumArguments);
			ArgumentBuffer->SetTextureNames(ParamTextureNames);
			ArgumentBuffer->SetTextureSizes(ParamTextureSizes);

			if (ParamValueBuffer != nullptr)
			{
				FUniformBufferPtr UniformBuffer = FRHI::Get()->CreateUniformBuffer(ParamValueBuffer->GetLength(), 1);

				ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(UpdateMIUniformBuffer,
					FUniformBufferPtr, UniformBuffer, UniformBuffer,
					TStreamPtr, ParamValueBuffer, ParamValueBuffer,
					{
						FRHI::Get()->UpdateHardwareResourceUB(UniformBuffer, ParamValueBuffer->GetBuffer());
					});

				ArgumentBuffer->SetBuffer(0, UniformBuffer);
			}

            if (!FVTSystem::IsEnabled())
            {
                // if vt system enabled, all texture will be in vt atlas.
                // do not need to load textures and put into argument buffer here.
                TI_ASSERT(ParamTextures.size() == ParamTextureNames.size());
                for (int32 i = 0; i < (int32)ParamTextures.size(); ++ i)
                {
                    TI_ASSERT(ParamTextures[i]->TextureResource != nullptr);
                    ArgumentBuffer->SetTexture(i + NumBuffers, ParamTextures[i]->TextureResource);
                }
                
                FShaderPtr MaterialShader = LinkedMaterial->GetDesc().Shader->ShaderResource;
                
                ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(UpdateMIArgumentBuffer,
                                                           FArgumentBufferPtr, ArgumentBuffer, ArgumentBuffer,
                                                           FShaderPtr, MaterialShader, MaterialShader,
                                                           {
                                                               FRHI::Get()->UpdateHardwareResourceAB(ArgumentBuffer, MaterialShader);
                                                           });
            }
		}
	}

	void TMaterialInstance::DestroyRenderThreadResource()
	{
		if (ArgumentBuffer != nullptr)
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(TMIDestroyArgumentBuffer,
				FArgumentBufferPtr, ArgumentBuffer, ArgumentBuffer,
				{
					ArgumentBuffer = nullptr;
				});
			ArgumentBuffer = nullptr;
		}
	}

	static const int32 ParamTypeLength[MIPT_COUNT] =
	{
		0,	//MIPT_UNKNOWN,
		4,	//MIPT_INT,
		4,	//MIPT_FLOAT,
		16,	//MIPT_INT4,
		16,	//MIPT_FLOAT4,
		8,	//MIPT_TEXTURE ((int32) + (int16) * 2),
	};

	int32 TMaterialInstance::GetParamTypeBytes(E_MI_PARAM_TYPE Type)
	{
		return ParamTypeLength[Type];
	}
}
