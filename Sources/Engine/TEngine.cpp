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

	void TEngine::InitEngine(const TEngineConfiguration& Config)
	{
		if (s_engine == nullptr)
		{
			s_engine = ti_new TEngine;
			s_engine->Init(Config);
		}
	}

	void TEngine::Init(const TEngineConfiguration& Config)
	{
		// Create device
		TI_ASSERT(Device == nullptr);
		Device = TDevice::CreateDevice(Config.Name, Config.Width, Config.Height);
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
		: Device(nullptr)
	{
	}

	TEngine::~TEngine()
	{
		TDevice::DestoryDevice(Device);
	}

	TDevice* TEngine::GetDevice()
	{
		return nullptr;
	}

	void TEngine::Start()
	{
		TI_ASSERT(Device);
		while (Device->Run())
		{

		}

		TEngine::Destroy();
	}
}