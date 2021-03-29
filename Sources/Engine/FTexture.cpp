/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	FTexture::FTexture()
		: FRenderResource(RRT_TEXTURE)
	{
	}

	FTexture::FTexture(const TTextureDesc& Desc)
		: FRenderResource(RRT_TEXTURE)
	{
		TextureDesc = Desc;
	}

	FTexture::~FTexture()
	{
		TI_ASSERT(IsRenderThread());
	}

	void FTexture::InitTextureInfo(TTexturePtr InTexture)
	{
		TextureDesc = InTexture->GetDesc();
	}
}