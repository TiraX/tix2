/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FTextureDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FTextureDx12::FTextureDx12()
		: FTexture(ERF_Dx12)
		, TexDescriptor(uint32(-1))
	{
	}

	FTextureDx12::FTextureDx12(const TTextureDesc& Desc)
		: FTexture(ERF_Dx12, Desc)
		, TexDescriptor(uint32(-1))
	{
	}

	FTextureDx12::~FTextureDx12()
	{
		Destroy();
	}

	void FTextureDx12::Destroy()
	{
		TI_ASSERT(IsRenderThread());
		if (TexDescriptor != uint32(-1))
		{
			FRHIDx12 * RHIDx12 = static_cast<FRHIDx12*>(FRHI::Get());
			RHIDx12->RecallDescriptor(EHT_CBV_SRV_UAV, TexDescriptor);
			TexDescriptor = uint32(-1);
		}
	}
}

#endif	// COMPILE_WITH_RHI_DX12