/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "TEngineResources.h"

namespace tix
{
	TTexturePtr TEngineResources::EmptyTextureWhite = nullptr;
	TTexturePtr TEngineResources::EmptyTextureBlack = nullptr;
	TTexturePtr TEngineResources::EmptyTextureNormal = nullptr;

	void TEngineResources::CreateGlobalResources()
	{
		const int32 W = 2;
		const int32 H = 2;
		const uint32 WhiteData[W * H] = { 0xfff0f0f0, 0xfff0f0f0, 0xfff0f0f0, 0xfff0f0f0 };
		const uint32 BlackData[W * H] = { 0xff000000, 0xff000000, 0xff000000, 0xff000000 };
		const uint32 NormalData[W * H] = { 0xff8080ff, 0xff8080ff, 0xff8080ff, 0xff8080ff };
		// Empty Textures
		TTextureDesc Desc;
		Desc.Type = ETT_TEXTURE_2D;
		Desc.Format = EPF_RGBA8;
		Desc.Width = W;
		Desc.Height = H;
		Desc.AddressMode = ETC_CLAMP_TO_EDGE;
		Desc.SRGB = 0;
		Desc.Mips = 1;

		TI_ASSERT(EmptyTextureWhite == nullptr);
		EmptyTextureWhite = ti_new TTexture(Desc);
		EmptyTextureWhite->AddSurface(W, H, (const uint8*)WhiteData, W * 4, sizeof(WhiteData));
		EmptyTextureWhite->InitRenderThreadResource();
		EmptyTextureBlack = ti_new TTexture(Desc);
		EmptyTextureBlack->AddSurface(W, H, (const uint8*)BlackData, W * 4, sizeof(BlackData));
		EmptyTextureBlack->InitRenderThreadResource();
		EmptyTextureNormal= ti_new TTexture(Desc);
		EmptyTextureNormal->AddSurface(W, H, (const uint8*)NormalData, W * 4, sizeof(NormalData));
		EmptyTextureNormal->InitRenderThreadResource();
	}

	void TEngineResources::DestroyGlobalResources()
	{
		EmptyTextureWhite = nullptr;
		EmptyTextureBlack = nullptr;
		EmptyTextureNormal = nullptr;
	}
}
