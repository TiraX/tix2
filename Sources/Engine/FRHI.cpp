/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHI.h"
#include "FRenderTarget.h"
#include "FRHIDx12.h"

namespace tix
{
	FRHI* FRHI::RHI = nullptr;

	FRHI* FRHI::Get()
	{
		return RHI;
	}

	void FRHI::CreateRHI(E_RHI_TYPE RHIType)
	{
		TI_ASSERT(RHI == nullptr);
#if defined (TI_PLATFORM_WIN32) && (COMPILE_WITH_RHI_DX12)
		RHI = ti_new FRHIDx12;
#elif defined (TI_PLATFORM_IOS) && (COMPILE_WITH_RHI_METAL)
        TI_ASSERT(0);
#else
#error("No avaible RHI for this platform.")
#endif
		RHI->InitRHI();
	}

	void FRHI::ReleaseRHI()
	{
		TI_ASSERT(RHI != nullptr);
		ti_delete RHI;
		RHI = nullptr;
	}

	FRHI::FRHI(E_RHI_TYPE InRHIType)
		: RHIType(InRHIType)
	{
		for (int32 i = 0; i < FRHIConfig::FrameBufferNum; ++i)
		{
			FrameResources[i] = nullptr;
		}
	}

	FRHI::~FRHI()
	{
	}

	void FRHI::SetViewport(const FViewport& InViewport)
	{
		Viewport = InViewport;
	}
	
	FRenderTargetPtr FRHI::CreateRenderTarget(int32 W, int32 H)
	{
		return ti_new FRenderTarget(W, H);
	}

	FRenderResourceTablePtr FRHI::CreateRenderResourceTable(uint32 InSize)
	{
		return ti_new FRenderResourceTable(InSize);
	}

	void FRHI::PushRenderTarget(FRenderTargetPtr RT)
	{
		RenderTargets.push_back(RT);
		RtViewports.push_back(Viewport);

		const vector2di& d = RT->GetDemension();
		SetViewport(FViewport(0, 0, d.X, d.Y));
	}

	FRenderTargetPtr FRHI::PopRenderTarget()
	{
		TI_ASSERT(RenderTargets.size() > 0);

		RenderTargets.pop_back();
		const FViewport& vp = RtViewports.back();
		SetViewport(vp);
		RtViewports.pop_back();

		if (RenderTargets.size() == 0)
			return nullptr;
		else
			return RenderTargets.back();
	}

	bool FRHI::UpdateHardwareResource(FRenderTargetPtr RenderTarget)
	{
		// Create render target render resource tables
		// Color buffers
		int32 ColorBufferCount = RenderTarget->GetColorBufferCount();
		TI_ASSERT(RenderTarget->RTColorTable == nullptr);
		RenderTarget->RTColorTable = RenderResourceHeap[EHT_RENDERTARGET].AllocateTable(ColorBufferCount);
		for (int32 i = 0 ; i < ColorBufferCount ; ++ i)
		{
			const FRenderTarget::RTBuffer& ColorBuffer = RenderTarget->GetColorBuffer(i);
			RenderTarget->RTColorTable->PutRTColorInTable(ColorBuffer.Texture, i);
		}

		// Depth stencil buffers
		{
			const FRenderTarget::RTBuffer& DepthStencilBuffer = RenderTarget->GetDepthStencilBuffer();
			FTexturePtr DSBufferTexture = DepthStencilBuffer.Texture;
			if (DSBufferTexture != nullptr)
			{
				TI_ASSERT(RenderTarget->RTDepthTable == nullptr);
				RenderTarget->RTDepthTable = RenderResourceHeap[EHT_DEPTHSTENCIL].AllocateTable(1);
				RenderTarget->RTDepthTable->PutRTDepthInTable(DSBufferTexture, 0);
			}
		}
		return true;
	}
}
