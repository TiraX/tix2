/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "TEngine.h"
#include "TVersion.h"
#include "TConsoleVariable.h"
#include "FRenderThread.h"

#if DEBUG_OPERATOR_NEW
void * operator new (std::size_t count)
{
	void* a = malloc(count);
	return a;
}
#endif

namespace tix
{
	TEngine* TEngine::s_engine = nullptr;
	E_Platform TEngine::CurrentPlatform = EP_Unknown;

	void TEngine::InitEngine(const TEngineConfiguration& Config)
	{
		if (s_engine == nullptr)
		{
			_LOG(Log, "TiX Engine v2.0.0\n");
			s_engine = ti_new TEngine;
			s_engine->Init(Config);
		}
	}

	void TEngine::Init(const TEngineConfiguration& Config)
	{
		TThread::IndicateGameThread();

#ifdef TI_PLATFORM_WIN32
		CurrentPlatform = EP_Windows;
#elif defined (TI_PLATFORM_IOS)
		CurrentPlatform = EP_IOS;
#elif defined (TI_PLATFORM_ANDROID)
		CurrentPlatform = EP_Android;
#else
		_LOG(Error, "Unknown platform.");
		TI_ASSERT(0);
#endif

		// Init CVar system and load ini configuration
		TConsoleVariables::Init();

		// Create device
		TI_ASSERT(Device == nullptr);
		Device = TDevice::CreateDevice(Config.Name, Config.Width, Config.Height);
		
		// Create Render Thread
		FRenderThread::CreateRenderThread();

		// Create components
		TI_ASSERT(Scene == nullptr);
		Scene = ti_new TScene;
		ResourceLibrary = ti_new TResourceLibrary;

		// Waiting for render thread create
		while (!FRenderThread::IsInited())
		{
			TThread::ThreadSleep(10);
		}
	}

	void TEngine::Destroy()
	{
		TI_ASSERT(s_engine);
		SAFE_DELETE(s_engine);
		TConsoleVariables::Destroy();
	}

	TEngine* TEngine::Get()
	{
		return s_engine;
	}

	TEngine::TEngine()
		: Device(nullptr)
		, Scene(nullptr)
		, ResourceLibrary(nullptr)
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

		// delete components
		SAFE_DELETE(Scene);
		SAFE_DELETE(ResourceLibrary);

		// Shut down render thread
		FRenderThread::DestroyRenderThread();

		// delete device
		TDevice::DestoryDevice(Device);
	}

	TDevice* TEngine::GetDevice()
	{
		return Device;
	}

	void TEngine::AddDefaultRenderer()
	{
		FRenderer * Renderer = ti_new FDefaultRenderer;
		AddRenderer(Renderer);
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

		// update inputs
		Device->GetInput()->UpdateEvents(Dt);

		Scene->TickAllNodes(Dt);

		for (auto T : Tickers)
		{
			T->Tick(Dt);
		}

		// Remember last frame start time
		LastFrameTime = CurrentFrameTime;

		// Tick finished, trigger render thread
		TickFinished();

		// Check if reach max frame interval time
		uint32 TimePast = (uint32)(TTimer::GetCurrentTimeMillis() - CurrentFrameTime);
		if (TimePast < FrameInterval)
		{
			TThread::ThreadSleep(FrameInterval - TimePast);
		}
	}

	void TEngine::TickFinished()
	{
		FRenderThread * RT = FRenderThread::Get();
		while (RT->GetTriggerNum() >= FRHIConfig::FrameBufferNum)
		{
			TThread::ThreadSleep(10);
		}

		RT->TriggerRender();
	}
}