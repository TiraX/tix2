/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	FTexture::FTexture()
		: FRenderResourceInHeap(EHT_TEXTURE)
	{
	}

	FTexture::FTexture(const TTextureDesc& Desc)
		: FRenderResourceInHeap(EHT_TEXTURE)
	{
		TextureDesc = Desc;
	}

	FTexture::~FTexture()
	{
	}

	void FTexture::InitTextureInfo(TTexturePtr InTexture)
	{
		TextureDesc = InTexture->GetDesc();
	}
}