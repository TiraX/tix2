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

	FRenderTarget::FRenderTarget(int32 W, int32 H)
		: Demension(W, H)
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
		Desc.AddressMode = ETC_CLAMP_TO_EDGE;

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
		//Buffer.RTResource = ti_new FRenderTargetResource(EHT_RENDERTARGET);
		Buffer.Texture->SetTextureFlag(ETF_RT_COLORBUFFER, true);
		Buffer.BufferIndex = ColorBufferIndex;

#if defined (TIX_DEBUG)
		{
			int8 NameBuf[64];
			sprintf_s(NameBuf, 64, "-CB%d", ColorBufferIndex);
			Texture->SetResourceName(GetResourceName() + NameBuf);
		}
#endif

		RTColorBuffers[ColorBufferIndex] = Buffer;
	}

	void FRenderTarget::AddDepthStencilBuffer(FTexturePtr Texture)
	{
		RTDepthStencilBuffer.Texture = Texture;
		//RTDepthStencilBuffer.RTResource = ti_new FRenderTargetResource(EHT_DEPTHSTENCIL);
	}

	void FRenderTarget::AddDepthStencilBuffer(E_PIXEL_FORMAT Format)
	{
		TTextureDesc Desc;
		Desc.Format = Format;
		Desc.Width = Demension.X;
		Desc.Height = Demension.Y;
		Desc.AddressMode = ETC_CLAMP_TO_EDGE;
		
		FRHI * RHI = FRHI::Get();
		FTexturePtr Texture = RHI->CreateTexture(Desc);
		Texture->SetTextureFlag(ETF_RT_DSBUFFER, true);
#if defined (TIX_DEBUG)
		Texture->SetResourceName(GetResourceName() + "-DS");
#endif

		RTDepthStencilBuffer.Texture = Texture;
		//RTDepthStencilBuffer.RTResource = ti_new FRenderTargetResource(EHT_DEPTHSTENCIL);
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
			
			TI_ASSERT(ColorBuffer.Texture != nullptr);
			RHI->UpdateHardwareResource(ColorBuffer.Texture);
		}

		if (RTDepthStencilBuffer.Texture != nullptr)
		{
			RHI->UpdateHardwareResource(RTDepthStencilBuffer.Texture);
		}

		RHI->UpdateHardwareResource(this);
	}
}