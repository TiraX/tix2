/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHI.h"
#include "FRHIDx12.h"

namespace tix
{
	FRHI* FRHI::CreateRHI(E_RHI_TYPE RHIType)
	{
#if defined (TI_PLATFORM_WIN32) && (COMPILE_WITH_RHI_DX12)
		return ti_new FRHIDx12;
#else
#error("No avaible RHI for this platform.")
#endif
		return nullptr;
	}

	FRHI::FRHI()
	{
	}

	FRHI::~FRHI()
	{
	}
}