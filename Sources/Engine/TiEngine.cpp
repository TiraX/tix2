/*
	Copyright (C) 2012
	By Zhao Shuai tirax.cn@gmail.com 2012.7.16
*/

#include "stdafx.h"

#include "TiEngine.h"
#include "TiVersion.h"
namespace ti
{
	TiEngine* TiEngine::s_engine = nullptr;

	void TiEngine::Create(int w, int h, void* handle, const char* windowName, uint32 option)
	{
		if (s_engine == nullptr)
		{
			s_engine = ti_new TiEngine;

//			TiDevice* device = s_engine->CreateDevice(w, h, handle, windowName);
		}
	}

	void TiEngine::InitComponent()
	{
		TiEngine* engine = TiEngine::Get();
		//engine->Log = ti_new TiLog;
		//engine->Log->Log("TiX engine. Ver %d.%d.%d.%d\n", k_major_version, k_mid_version, k_minor_version, k_build_version);
	}

	void TiEngine::InitGraphics(void* param)
	{
		TiEngine* engine = TiEngine::Get();
	}

	void TiEngine::Destroy()
	{
		TI_ASSERT(s_engine);
		SAFE_DELETE(s_engine);
	}

	TiEngine* TiEngine::Get()
	{
		return s_engine;
	}

	TiEngine::TiEngine()
	{
	}

	TiEngine::~TiEngine()
	{
	}
}
