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
	FTextureDx12::FTextureDx12(TImagePtr InSourceImage)
		: FTexture(ERF_Dx12, InSourceImage)
	{
	}

	FTextureDx12::FTextureDx12(E_PIXEL_FORMAT InFormat, int32 InWidth, int32 InHeight)
		: FTexture(ERF_Dx12, InFormat, InWidth, InHeight)
	{
	}

	FTextureDx12::~FTextureDx12()
	{
		FRHIDx12 * RHIDx12 = static_cast<FRHIDx12*>(FRHI::Get());
		if (IsRenderThread())
		{
			RHIDx12->RecallDescriptor(TexDescriptor);
		}
		else
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TextureRecallDescriptor,
				FRHIDx12*, RHIDx12, RHIDx12,
				D3D12_CPU_DESCRIPTOR_HANDLE, Handle, TexDescriptor,
				{
					RHIDx12->RecallDescriptor(Handle);
				});
		}
	}
}

#endif	// COMPILE_WITH_RHI_DX12