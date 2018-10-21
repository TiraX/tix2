/*
TiX Engine v2.0 Copyright (C) 2018
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
		if (ParamValueBuffer != nullptr)
		{
			TI_ASSERT(UniformBuffer == nullptr);
			UniformBuffer = FRHI::Get()->CreateUniformBuffer(ParamValueBuffer->GetLength());
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(UpdateMIUniformBuffer,
				FUniformBufferPtr, UniformBuffer, UniformBuffer,
				TStreamPtr, UniformBufferData, ParamValueBuffer,
				{
					RHI->UpdateHardwareResource(UniformBuffer, UniformBufferData->GetBuffer());
				});
		}
		if (ParamTextures.size() > 0)
		{
			TI_ASSERT(TextureResourceTable == nullptr);
			TextureResourceTable = FRHI::Get()->CreateRenderResourceTable((uint32)ParamTextures.size());
			TVector<FTexturePtr> Textures;
			for (auto Tex : ParamTextures)
			{
				TI_ASSERT(Tex->TextureResource != nullptr);
				Textures.push_back(Tex->TextureResource);
			}
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(UpdateMITextureResourceTable,
				FRenderResourceTablePtr, MI_TextureResourceTable, TextureResourceTable,
				TVector<FTexturePtr>, Textures, Textures,
				{
					RHI->GetRenderResourceHeap(EHT_TEXTURE).AllocateTable(MI_TextureResourceTable);
					for (int32 t = 0; t < (int32)Textures.size(); ++t)
					{
						FTexturePtr Texture = Textures[t];
						MI_TextureResourceTable->PutTextureInTable(Texture, t);
					}
				});
		}
	}

	void TMaterialInstance::DestroyRenderThreadResource()
	{
		TI_TODO("Implement here.");
		UniformBuffer = nullptr;
		TextureResourceTable = nullptr;
	}

	static const int32 ParamTypeLength[MIPT_COUNT] =
	{
		0,	//MIPT_UNKNOWN,
		4,	//MIPT_INT,
		4,	//MIPT_FLOAT,
		16,	//MIPT_INT4,
		16,	//MIPT_FLOAT4,
		4,	//MIPT_TEXTURE (int),
	};

	int32 TMaterialInstance::GetParamTypeBytes(E_MI_PARAM_TYPE Type)
	{
		return ParamTypeLength[Type];
	}
}
