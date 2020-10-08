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

	/////////////////////////////////////////////////////////
	FTextureReadableDx12::FTextureReadableDx12()
	{
	}

	FTextureReadableDx12::FTextureReadableDx12(const TTextureDesc& Desc)
		: FTextureDx12(Desc)
	{
	}

	FTextureReadableDx12::~FTextureReadableDx12()
	{
		TI_ASSERT(IsRenderThread());
	}

	TImagePtr FTextureReadableDx12::ReadTextureData()
	{
		if (ReadbackResource.GetResource() != nullptr)
		{
			const int32 TextureDataSize = TImage::GetDataSize(TextureDesc.Format, TextureDesc.Width, TextureDesc.Height);
			D3D12_RANGE ReadbackBufferRange{ 0, (SIZE_T)TextureDataSize };
			uint8* Result = nullptr;
			HRESULT Hr = ReadbackResource.GetResource()->Map(0, &ReadbackBufferRange, reinterpret_cast<void**>(&Result));
			TI_ASSERT(SUCCEEDED(Hr));

			TImagePtr Image = ti_new TImage(TextureDesc.Format, TextureDesc.Width, TextureDesc.Height);
			uint8* ImageData = Image->Lock();
			memcpy(ImageData, Result, TextureDataSize);
			Image->Unlock();

			// Code goes here to access the data via pReadbackBufferData.
			D3D12_RANGE EmptyRange{ 0, 0 };
			ReadbackResource.GetResource()->Unmap(0, &EmptyRange);
			return Image;
		}
		return nullptr;
	}
}

#endif	// COMPILE_WITH_RHI_DX12