/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/


#include "stdafx.h"

#include "TDevice.h"
#include "TDeviceWin32.h"
#include "TDeviceIOS.h"

namespace tix
{
	TDevice* TDevice::CreateDevice(const TString& Name, int32 Width, int32 Height)
	{
		TDevice* device = nullptr;
#ifdef TI_PLATFORM_WIN32
		device = ti_new TDeviceWin32(Width, Height, nullptr, Name.c_str());
#elif defined (TI_PLATFORM_IOS)
		device = ti_new TDeviceIOS(Width, Height);
#elif defined (TI_PLATFORM_ANDROID)
		device = ti_new TiAndroidDevice(w, h);
#else
		TI_ASSERT(0);
#endif
		return device;
	}

	void TDevice::DestoryDevice(TDevice* Device)
	{
		ti_delete Device;
	}

	TDevice::TDevice(int32 w, int32 h)
		: Width(w)
		, Height(h)
	{
		Input	= ti_new TInput;
	}

	TDevice::~TDevice()
	{
		ti_delete Input;
	}

	void TDevice::Resize(int32 w, int32 h)
	{
		Width	= w;
		Height	= h;
	}
}
