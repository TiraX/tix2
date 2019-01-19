/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIMetal.h"
#include "FTextureMetal.h"

#if COMPILE_WITH_RHI_METAL

namespace tix
{
	FTextureMetal::FTextureMetal()
	{
        Texture = nil;
	}

	FTextureMetal::FTextureMetal(const TTextureDesc& Desc)
		: FTexture(Desc)
	{
	}

	FTextureMetal::~FTextureMetal()
	{
	}
}

#endif	// COMPILE_WITH_RHI_METAL
