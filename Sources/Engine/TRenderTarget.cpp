/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TRenderTarget.h"

namespace tix
{
	TRenderTargetPtr TRenderTarget::Create(int32 W, int32 H)
	{
		return ti_new TRenderTarget(W, H);
	}

	TRenderTarget::TRenderTarget(int32 W, int32 H)
		: TResource(ERES_RENDER_TARGET)
		, Demension(W, H)
	{}

	TRenderTarget::~TRenderTarget()
	{
	}

	void TRenderTarget::InitRenderThreadResource()
	{
		TI_ASSERT(RTResource == nullptr);
		RTResource = FRHI::Get()->CreateRenderTarget();

		// Init RT render resource
		int32 ValidColorBuffers = 0;
		for (int32 i = 0 ; i < ERTC_COUNT ; ++ i)
		{
			const RTBuffer& ColorBuffer = RTColorBuffers[i];
			// Color buffer must be continous
			if (ColorBuffer.BufferIndex == ERTC_INVALID)
				break;

			if (ColorBuffer.BufferType == ERTAT_TEXTURE)
			{
				ColorBuffer.Texture->InitRenderThreadResource();
				++ValidColorBuffers;
			}
		}
		RTResource->SetValidColorBufferCount(ValidColorBuffers);

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TRenderTargetUpdateFRT,
			FRenderTargetPtr, RenderTarget_RT, RTResource,
			TRenderTargetPtr, RTDesc, this,
			{
				RHI->UpdateHardwareResource(RenderTarget_RT, RTDesc);
			});
	}

	void TRenderTarget::DestroyRenderThreadResource()
	{
		if (RTResource != nullptr)
		{
			// Release RTResource in render thread
			for (auto& ColorBuffer : RTColorBuffers)
			{
				if (ColorBuffer.BufferType == ERTAT_TEXTURE)
				{
					ColorBuffer.Texture->DestroyRenderThreadResource();
					ColorBuffer.Texture = nullptr;
				}
			}

			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(TRenderTargetDestroyFRenderTarget,
				FRenderTargetPtr, RenderTarget_RT, RTResource,
				{
					RenderTarget_RT->Destroy();
				});

			RTResource = nullptr;
		}
	}

	void TRenderTarget::AddColorBuffer(E_PIXEL_FORMAT Format, E_RT_COLOR_BUFFER ColorBufferIndex)
	{
		TTextureDesc Desc;
		Desc.Format = Format;
		Desc.Width = Demension.X;
		Desc.Height = Demension.Y;
		Desc.WrapMode = ETC_CLAMP_TO_EDGE;

		TTexturePtr Texture = ti_new TTexture(Desc);
		AddColorBuffer(Texture, ColorBufferIndex);
	}

	void TRenderTarget::AddColorBuffer(TTexturePtr Texture, E_RT_COLOR_BUFFER ColorBufferIndex)
	{
		RTBuffer Buffer;
		Buffer.Texture = Texture;
		Buffer.BufferIndex = ColorBufferIndex;
		Buffer.BufferType = ERTAT_TEXTURE;
		Buffer.Level = 0;

		RTColorBuffers[ColorBufferIndex] = Buffer;
	}

	void TRenderTarget::AddDepthStencilBuffer(E_PIXEL_FORMAT Format)
	{
		TI_ASSERT(0);
	}

	void TRenderTarget::Compile()
	{
		DestroyRenderThreadResource();
		InitRenderThreadResource();
	}
}
