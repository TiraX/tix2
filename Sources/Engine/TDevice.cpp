/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/


#include "stdafx.h"

#include "TDevice.h"

namespace tix
{
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
