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
		for (auto& Attachment : RtAttachments)
		{
			if (Attachment.AttachType == ERTAT_TEXTURE)
			{
				Attachment.Texture->InitRenderThreadResource();
			}
		}

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
			for (auto& Attachment : RtAttachments)
			{
				if (Attachment.AttachType == ERTAT_TEXTURE)
				{
					Attachment.Texture->DestroyRenderThreadResource();
					Attachment.Texture = nullptr;
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

	void TRenderTarget::AddTextureAttachment(E_PIXEL_FORMAT Format, E_RT_ATTACH Attachment)
	{
		TTextureDesc Desc;
		Desc.Format = Format;
		Desc.Width = Demension.X;
		Desc.Height = Demension.Y;
		Desc.WrapMode = ETC_CLAMP_TO_EDGE;

		TTexturePtr Texture = ti_new TTexture(Desc);
		AddAttachment(Texture, Attachment);
	}

	void TRenderTarget::AddAttachment(TTexturePtr Texture, E_RT_ATTACH Attachment)
	{
		RTAttachment att;
		att.Texture = Texture;
		att.Attachment = Attachment;
		att.AttachType = ERTAT_TEXTURE;
		att.Level = 0;

		RtAttachments.push_back(att);
	}

	void TRenderTarget::Compile()
	{
		DestroyRenderThreadResource();
		InitRenderThreadResource();
	}
}
