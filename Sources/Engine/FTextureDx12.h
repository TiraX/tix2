/*
	TiX Engine v2.0 Copyright (C) 2018
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

		virtual void Destroy() override;
	protected:

	private:
		FGPUResourceDx12 TextureResource;
		uint32 TexDescriptor;
		friend class FRHIDx12;
	};
}

#endif	// COMPILE_WITH_RHI_DX12
