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
			ArgumentBuffer = FRHI::Get()->CreateArgumentBuffer((int32)ParamTextureNames.size());
			ArgumentBuffer->SetTextureNames(ParamTextureNames);
			ArgumentBuffer->SetTextureSizes(ParamTextureSizes);

			if (ParamValueBuffer != nullptr)
			{
				ArgumentBuffer->SetDataBuffer(ParamValueBuffer->GetBuffer(), ParamValueBuffer->GetLength());
			}

			TI_ASSERT(ParamTextures.size() == ParamTextureNames.size());
			for (int32 i = 0 ; i < (int32)ParamTextures.size(); ++ i)
			{
				TI_ASSERT(ParamTextures[i]->TextureResource != nullptr);
				ArgumentBuffer->SetTexture(i, ParamTextures[i]->TextureResource);
			}

			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(UpdateMIArgumentBuffer,
				FArgumentBufferPtr, ArgumentBuffer, ArgumentBuffer,
				{
					FRHI::Get()->UpdateHardwareResourceAB(ArgumentBuffer);
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
