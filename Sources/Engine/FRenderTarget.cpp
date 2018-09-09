/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRenderTarget.h"

namespace tix
{
	FRenderTargetPtr FRenderTarget::Create(int32 W, int32 H)
	{
		FRHI * RHI = FRHI::Get();
		return RHI->CreateRenderTarget(W, H);
	}

	FRenderTarget::FRenderTarget(E_RESOURCE_FAMILY InFamily, int32 W, int32 H)
		: FRenderResource(InFamily)
		, Demension(W, H)
		, ColorBuffers(0)
	{
	}

	FRenderTarget::~FRenderTarget()
	{
	}

	void FRenderTarget::AddColorBuffer(E_PIXEL_FORMAT Format, E_RT_COLOR_BUFFER ColorBufferIndex)
	{
		TTextureDesc Desc;
		Desc.Format = Format;
		Desc.Width = Demension.X;
		Desc.Height = Demension.Y;
		Desc.WrapMode = ETC_CLAMP_TO_EDGE;

		FRHI * RHI = FRHI::Get();
		FTexturePtr Texture = RHI->CreateTexture(Desc);
		AddColorBuffer(Texture, ColorBufferIndex);
	}

	void FRenderTarget::AddColorBuffer(FTexturePtr Texture, E_RT_COLOR_BUFFER ColorBufferIndex)
	{
		if (RTColorBuffers[ColorBufferIndex].BufferIndex == ERTC_INVALID)
		{
			++ColorBuffers;
		}

		RTBuffer Buffer;
		Buffer.Texture = Texture;
		Buffer.Texture->SetTextureFlag(ETF_RT_COLORBUFFER, true);
		Buffer.BufferIndex = ColorBufferIndex;
		Buffer.BufferType = ERTAT_TEXTURE;
		Buffer.Level = 0;

#if defined (TIX_DEBUG)
		{
			int8 NameBuf[64];
			sprintf_s(NameBuf, 64, "-CB%d", ColorBufferIndex);
			Texture->SetResourceName(GetResourceName() + NameBuf);
		}
#endif

		RTColorBuffers[ColorBufferIndex] = Buffer;
	}

	void FRenderTarget::AddDepthStencilBuffer(E_PIXEL_FORMAT Format)
	{
		TTextureDesc Desc;
		Desc.Format = Format;
		Desc.Width = Demension.X;
		Desc.Height = Demension.Y;
		Desc.WrapMode = ETC_CLAMP_TO_EDGE;
		
		FRHI * RHI = FRHI::Get();
		FTexturePtr Texture = RHI->CreateTexture(Desc);
		Texture->SetTextureFlag(ETF_RT_DSBUFFER, true);
#if defined (TIX_DEBUG)
		Texture->SetResourceName(GetResourceName() + "-DS");
#endif

		RTDepthStencilBuffer.Texture = Texture;
		RTDepthStencilBuffer.BufferType = ERTAT_TEXTURE;
		RTDepthStencilBuffer.Level = 0;
	}

	void FRenderTarget::Compile()
	{
		TI_ASSERT(IsRenderThread());
		FRHI * RHI = FRHI::Get();

		for (int32 i = 0; i < ERTC_COUNT; ++i)
		{
			const RTBuffer& ColorBuffer = RTColorBuffers[i];
			// Color buffer must be continuous
			if (ColorBuffer.BufferIndex == ERTC_INVALID)
				break;

			if (ColorBuffer.BufferType == ERTAT_TEXTURE)
			{
				TI_ASSERT(ColorBuffer.Texture != nullptr);
				RHI->UpdateHardwareResource(ColorBuffer.Texture, nullptr);
			}
		}

		if (RTDepthStencilBuffer.Texture != nullptr)
		{
			RHI->UpdateHardwareResource(RTDepthStencilBuffer.Texture, nullptr);
		}

		RHI->UpdateHardwareResource(this);
	}
}