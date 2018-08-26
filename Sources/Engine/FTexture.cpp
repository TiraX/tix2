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

	FTexture::FTexture(E_RESOURCE_FAMILY InFamily, const TTextureDesc& Desc)
		: FRenderResource(InFamily)
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