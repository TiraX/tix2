/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	TShaderBinding::TShaderBinding()
		: TResource(ERES_SHADER_BINDING)
	{}

	TShaderBinding::~TShaderBinding()
	{
	}

	void TShaderBinding::InitRenderThreadResource()
	{
		TI_ASSERT(0);
		//TI_ASSERT(TextureResource == nullptr);
		//TextureResource = FRHI::Get()->CreateTexture();

		//ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TTextureUpdateFTexture,
		//	FTexturePtr, Texture_RT, TextureResource,
		//	TTexturePtr, TextureData, this,
		//	{
		//		Texture_RT->InitTextureInfo(TextureData);
		//		RHI->UpdateHardwareResource(Texture_RT, TextureData);
		//	});
	}

	void TShaderBinding::DestroyRenderThreadResource()
	{
		TI_ASSERT(0);
		//if (TextureResource != nullptr)
		//{
		//	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(TTextureDestroyFTexture,
		//		FTexturePtr, Texture_RT, TextureResource,
		//		{
		//			Texture_RT->Destroy();
		//		});
		//	TextureResource = nullptr;
		//}
	}
}
