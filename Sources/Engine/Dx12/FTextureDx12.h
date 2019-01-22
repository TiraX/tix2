/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12
#include "FGPUResourceDx12.h"

namespace tix
{
	class FTextureDx12 : public FTexture
	{
	public:
		FTextureDx12();
		FTextureDx12(const TTextureDesc& Desc);
		virtual ~FTextureDx12();

	protected:

	private:
		FGPUResourceDx12 TextureResource;
		friend class FRHIDx12;
	};
}

#endif	// COMPILE_WITH_RHI_DX12