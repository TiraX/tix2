/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	FTexture::FTexture()
	{
	}

	FTexture::FTexture(const TTextureDesc& Desc)
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