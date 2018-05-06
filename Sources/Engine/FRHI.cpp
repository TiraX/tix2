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
		for (int32 i = 0; i < FrameBufferNum; ++i)
		{
			FrameResources[i] = nullptr;
		}
	}

	FRHI::~FRHI()
	{
	}
}