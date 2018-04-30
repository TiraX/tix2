/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "TEngine.h"
#include "TVersion.h"
#include "FRenderThread.h"

namespace tix
{
	TEngine* TEngine::s_engine = nullptr;

	void TEngine::InitEngine(const TEngineConfiguration& Config)
	{
		if (s_engine == nullptr)
		{
			_LOG("TiX Engine v2.0.0\n");
			s_engine = ti_new TEngine;
			s_engine->Init(Config);
		}
	}

	void TEngine::Init(const TEngineConfiguration& Config)
	{
		TThread::IndicateGameThread();

		// Create device
		TI_ASSERT(Device == nullptr);
		Device = TDevice::CreateDevice(Config.Name, Config.Width, Config.Height);

		// Create Render Thread
		TI_ASSERT(RenderThread == nullptr);
		RenderThread = ti_new FRenderThread();
		RenderThread->Start();
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
		, RenderThread(nullptr)
	{
	}

	TEngine::~TEngine()
	{
		// Remove all tickers
		for (auto T : Tickers)
		{
			ti_delete T;
		}
		Tickers.clear();

		// Shut down render thread
		RenderThread->Stop();
		ti_delete RenderThread;
		RenderThread = nullptr;

		// delete device
		TDevice::DestoryDevice(Device);
	}

	TDevice* TEngine::GetDevice()
	{
		return Device;
	}

	FRenderThread* TEngine::GetRenderThread()
	{
		return RenderThread;
	}

	void TEngine::AddRenderer(FRenderer* Renderer)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(AddRendererInRenderThread, 
			FRenderer*, Renderer, Renderer,
			{
				RenderThread->AddRenderer(Renderer);
			});
	}

	void TEngine::AddTicker(TTicker* Ticker)
	{
		Tickers.push_back(Ticker);
	}

	static const uint64 FrameInterval = 33;

	void TEngine::Start()
	{
		TI_ASSERT(Device);

		LastFrameTime = TTimer::GetCurrentTimeMillis();

		while (Device->Run())
		{
			Tick();
		}

		TEngine::Destroy();
	}

	void TEngine::Tick()
	{
		uint64 CurrentFrameTime = TTimer::GetCurrentTimeMillis();
		uint32 Delta = (uint32)(CurrentFrameTime - LastFrameTime);
		float  Dt = Delta * 0.001f;
		_LOG("Tick %ld - %ld\n", CurrentFrameTime, LastFrameTime);

		for (auto T : Tickers)
		{
			T->Tick(Dt);
		}

		uint32 TimePast = (uint32)(TTimer::GetCurrentTimeMillis() - CurrentFrameTime);
		if (TimePast < FrameInterval)
		{
			TThread::ThreadSleep(FrameInterval - TimePast);
		}
		LastFrameTime = CurrentFrameTime;
	}
}