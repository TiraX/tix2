/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHI.h"
#include "FRenderTarget.h"
#include "Dx12/FRHIDx12.h"
#include "Metal/FRHIMetal.h"

namespace tix
{
	FRHI* FRHI::RHI = nullptr;

	FRHI* FRHI::Get()
	{
		return RHI;
	}

	void FRHI::CreateRHI()
	{
		TI_ASSERT(RHI == nullptr);
#if defined (TI_PLATFORM_WIN32) && (COMPILE_WITH_RHI_DX12)
		RHI = ti_new FRHIDx12;
#elif defined (TI_PLATFORM_IOS) && (COMPILE_WITH_RHI_METAL)
        RHI = ti_new FRHIMetal;
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
	
	//FRenderTargetPtr FRHI::CreateRenderTarget(int32 W, int32 H)
	//{
	//	return ti_new FRenderTarget(W, H);
	//}

	FRenderResourceTablePtr FRHI::CreateRenderResourceTable(uint32 InSize, E_RENDER_RESOURCE_HEAP_TYPE InHeap)
	{
		FRenderResourceTablePtr Table = ti_new FRenderResourceTable(InSize);
		GetRenderResourceHeap(InHeap).InitResourceTable(Table);
		return Table;
	}

	void FRHI::PushRenderTarget(FRenderTargetPtr RT, const int8* PassName)
	{
		RenderTargets.push_back(RT);
        
        const vector2di& d = RT->GetDemension();
        FViewport RTViewport(0, 0, d.X, d.Y);
		RtViewports.push_back(RTViewport);

		SetViewport(RTViewport);
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
}
