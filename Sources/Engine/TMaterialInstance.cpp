/*
TiX Engine v2.0 Copyright (C) 2018~2019
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
			ArgumentBuffer = FRHI::Get()->CreateArgumentBuffer(LinkedMaterial->GetDesc().Shader->ShaderResource);
			ArgumentBuffer->SetTextureNames(ParamTextureNames);
			ArgumentBuffer->SetTextureSizes(ParamTextureSizes);

			TVector<FTexturePtr> Textures;
			TI_ASSERT(ParamTextures.size() == ParamTextureNames.size());
			for (auto Tex : ParamTextures)
			{
				TI_ASSERT(Tex->TextureResource != nullptr);
				Textures.push_back(Tex->TextureResource);
			}

			ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(UpdateMIArgumentBuffer,
				FArgumentBufferPtr, ArgumentBuffer, ArgumentBuffer,
				TStreamPtr, UniformBufferData, ParamValueBuffer,
				TVector<FTexturePtr>, Textures, Textures,
				{
					FRHI::Get()->UpdateHardwareResource(ArgumentBuffer, UniformBufferData, Textures);
				});
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
