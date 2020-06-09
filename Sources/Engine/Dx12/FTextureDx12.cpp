/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FTextureDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FTextureDx12::FTextureDx12()
	{
	}

	FTextureDx12::FTextureDx12(const TTextureDesc& Desc)
		: FTexture(Desc)
	{
	}

	FTextureDx12::~FTextureDx12()
	{
	}
}

#endif	// COMPILE_WITH_RHI_DX12