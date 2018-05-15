/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	FTexture::FTexture(E_RESOURCE_FAMILY InFamily, TImagePtr InSourceImage)
		: FRenderResource(InFamily)
		, Format(InSourceImage->GetFormat())
		, Width(InSourceImage->GetWidth())
		, Height(InSourceImage->GetHeight())
	{
	}

	FTexture::FTexture(E_RESOURCE_FAMILY InFamily, E_PIXEL_FORMAT InFormat, int32 InWidth, int32 InHeight)
		: FRenderResource(InFamily)
		, Format(InFormat)
		, Width(InWidth)
		, Height(InHeight)
	{
	}

	FTexture::~FTexture()
	{
	}
}