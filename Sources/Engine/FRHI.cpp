/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHI.h"
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
#else
#error("No avaible RHI for this platform.")
#endif
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

	uint32 FRHI::AllocateHeapSlot(E_RENDER_RESOURCE_HEAP_TYPE Heap)
	{
		TI_ASSERT(0);
		return 0;
		//return RenderResourceHeap[Heap].AllocateSlot();
	}

	void FRHI::RecallHeapSlot(E_RENDER_RESOURCE_HEAP_TYPE Heap, uint32 SlotIndex)
	{
		RenderResourceHeap[Heap].RecallSlot(SlotIndex);
	}
}