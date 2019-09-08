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
	uint32 FRHI::NumGPUFrames = 0;

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

		CurrentCommandListState.Reset();
		CurrentCommandListCounter[EPL_GRAPHICS] = -1;
        CurrentCommandListCounter[EPL_COMPUTE] = -1;
        CurrentCommandListCounter[EPL_BLIT] = -1;
		ListExecuteOrder.clear();
	}

	FRHI::~FRHI()
	{
		CurrentRenderTarget = nullptr;
		CurrentBoundResource.Reset();
	}

	void FRHI::SetViewport(const FViewport& InViewport)
	{
		Viewport = InViewport;
	}
	
    void FRHI::BeginFrame()
    {
        CurrentCommandListState.Reset();
        CurrentCommandListCounter[EPL_GRAPHICS] = -1;
        CurrentCommandListCounter[EPL_COMPUTE] = -1;
        CurrentCommandListCounter[EPL_BLIT] = -1;
        ListExecuteOrder.clear();
        CurrentRenderTarget = nullptr;
        CurrentBoundResource.Reset();
    }
    
	FRenderResourceTablePtr FRHI::CreateRenderResourceTable(uint32 InSize, E_RENDER_RESOURCE_HEAP_TYPE InHeap)
	{
		FRenderResourceTablePtr Table = ti_new FRenderResourceTable(InSize);
		GetRenderResourceHeap(InHeap).InitResourceTable(Table);
		return Table;
	}

    void FRHI::BeginRenderToRenderTarget(FRenderTargetPtr RT, const int8* PassName)
	{
		CurrentRenderTarget = RT;
        
        const vector2di& d = RT->GetDemension();
		RtViewport.Left = 0;
		RtViewport.Top = 0;
		RtViewport.Width = d.X;
		RtViewport.Height = d.Y;

		SetViewport(RtViewport);
	}
}
