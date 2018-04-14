/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/


#include "stdafx.h"

#include "TDevice.h"
#include "TDeviceWin32.h"

namespace tix
{
	TDevice* TDevice::CreateDevice(const TString& Name, int Width, int Height)
	{
		TDevice* device = nullptr;
#ifdef TI_PLATFORM_WIN32
		device = ti_new TWin32Device(Width, Height, nullptr, Name.c_str());
#elif defined (TI_PLATFORM_IOS)
		TiIOSDevice::CheckFeatures();
		device = ti_new TiIOSDevice(w, h);
#elif defined (TI_PLATFORM_ANDROID)
		device = ti_new TiAndroidDevice(w, h);
#else
		TI_ASSERT(0);
		TI_TODO("implement other devices");
#endif
		return device;
	}

	void TDevice::DestoryDevice(TDevice* Device)
	{
		ti_delete Device;
	}

	TDevice::TDevice(int w, int h)
		: Width(w)
		, Height(h)
	{
		Input	= ti_new TInput;
	}

	TDevice::~TDevice()
	{
		ti_delete	Input;
	}

	void TDevice::Resize(int w, int h)
	{
		Width	= w;
		Height	= h;
	}
}
