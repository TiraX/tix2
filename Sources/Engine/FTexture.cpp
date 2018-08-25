/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	FTexture::FTexture(E_RESOURCE_FAMILY InFamily)
		: FRenderResource(InFamily)
	{
	}

	FTexture::FTexture(E_RESOURCE_FAMILY InFamily, E_PIXEL_FORMAT InFormat, int32 InWidth, int32 InHeight)
		: FRenderResource(InFamily)
	{
		TextureDesc.Format = InFormat;
		TextureDesc.Width = InWidth;
		TextureDesc.Height = InHeight;
	}

	FTexture::~FTexture()
	{
	}

	void FTexture::InitTextureInfo(TTexturePtr InTexture)
	{
		TextureDesc = InTexture->GetDesc();
	}
}