/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "TEngine.h"
#include "TVersion.h"

namespace tix
{
	TEngine* TEngine::s_engine = nullptr;

	void TEngine::Create(int w, int h, void* handle, const char* windowName, uint32 option)
	{
		if (s_engine == nullptr)
		{
			s_engine = ti_new TEngine;

//			TiDevice* device = s_engine->CreateDevice(w, h, handle, windowName);
		}
	}

	void TEngine::InitComponent()
	{
		TEngine* engine = TEngine::Get();
		//engine->Log = ti_new TiLog;
		//engine->Log->Log("TiX engine. Ver %d.%d.%d.%d\n", k_major_version, k_mid_version, k_minor_version, k_build_version);
	}

	void TEngine::InitGraphics(void* param)
	{
		TEngine* engine = TEngine::Get();
	}

	void TEngine::Destroy()
	{
		TI_ASSERT(s_engine);
		SAFE_DELETE(s_engine);
	}

	TEngine* TEngine::Get()
	{
		return s_engine;
	}

	TEngine::TEngine()
	{
	}

	TEngine::~TEngine()
	{
	}

	TDevice* TEngine::GetDevice()
	{
		return nullptr;
	}
}
