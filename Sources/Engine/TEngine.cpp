/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "TEngine.h"
#include "TVersion.h"
#include "TConsoleVariable.h"
#include "FRenderThread.h"
#if defined TI_PLATFORM_IOS
#import "TDirectorCaller.h"
#endif

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
    TAppInfo TEngine::AppInfo;

	void TEngine::InitEngine(const TEngineDesc& Config)
	{
		if (s_engine == nullptr)
		{
			// Init CVar system very first and load ini configuration
			TConsoleVariables::Init();

			_LOG(Log, "TiX Engine v2.0.0\n");
			s_engine = ti_new TEngine;
			s_engine->Init(Config);
		}
	}

	void TEngine::Init(const TEngineDesc& Config)
	{
		TThread::IndicateGameThread();
        
        AppInfo.Width = Config.Width;
        AppInfo.Height = Config.Height;

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
		// Create log system
		LogSystem = ti_new TLog;

		// Create device
		TI_ASSERT(Device == nullptr);
		Device = TDevice::CreateDevice(Config.Name, Config.Width, Config.Height);
		
		// Create Loading Thread
		TThreadLoading::CreateLoadingThread();

		// Create Render Thread
		FRenderThread::CreateRenderThread();

		// Create Global Resources
		TEngineResources::CreateGlobalResources();

		// Create components
		TI_ASSERT(Scene == nullptr);
		Scene = ti_new TScene;
		ResourceLibrary = ti_new TAssetLibrary;

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

	float TEngine::GameTime()
	{
		return s_engine->GameTimeElapsed;
	}

	TEngine::TEngine()
		: IsRunning(false)
		, LogSystem(nullptr)
		, Device(nullptr)
		, Scene(nullptr)
		, ResourceLibrary(nullptr)
		, LastFrameTime(0)
		, GameTimeElapsed(0.f)
		, MainThreadTasks(1024)
		, bFreezeTick(false)
		, bStepNext(false)
	{
	}

	TEngine::~TEngine()
	{
		TEngineResources::DestroyGlobalResources();

		// Remove all tickers
		for (auto T : Tickers)
		{
			ti_delete T;
		}
		Tickers.clear();

		// delete components
		SAFE_DELETE(Scene);
		SAFE_DELETE(ResourceLibrary);

		// Shut down loading thread
		TThreadLoading::DestroyLoadingThread();

		// Shut down render thread
		FRenderThread::DestroyRenderThread();

		// delete device
		TDevice::DestoryDevice(Device);

		SAFE_DELETE(LogSystem);
	}

	TDevice* TEngine::GetDevice()
	{
		return Device;
	}

	void TEngine::UseDefaultRenderer()
	{
		FRenderer * Renderer = ti_new FDefaultRenderer;
		AssignRenderer(Renderer);
	}

	void TEngine::AssignRenderer(FRenderer* Renderer)
	{
		ENQUEUE_RENDER_COMMAND(AddRendererInRenderThread)(
			[Renderer]()
			{
				FRenderThread::Get()->AssignRenderer(Renderer);
			});
	}

	void TEngine::AddTicker(TTicker* Ticker)
	{
		Tickers.push_back(Ticker);
	}

	void TEngine::FreezeTick()
	{
		bFreezeTick = !bFreezeTick;
	}

	void TEngine::TickStepNext()
	{
		bStepNext = true;
	}

	static const uint64 FrameInterval = 16;

	void TEngine::Start()
	{
		TI_ASSERT(Device);

		LastFrameTime = TTimer::GetCurrentTimeMillis();
		IsRunning = true;

#if defined (TI_PLATFORM_WIN32)
		while (IsRunning && Device->Run())
		{
			Tick();
		}

		TEngine::Destroy();
#elif defined (TI_PLATFORM_IOS)
        [[TDirectorCaller sharedDirectorCaller] startMainLoopWithInterval: 1.0f / 60.f];
#else
#error("Not supported platform")
#endif
	}

	void TEngine::Shutdown()
	{
#if defined (TI_PLATFORM_WIN32)
		IsRunning = false;
#else
#error("Not supported platform")
#endif
	}

	void TEngine::BeginFrame()
	{
		DoTasks();
		Scene->ResetActiveLists();
	}

	void TEngine::EndFrame()
	{
		// Tick finished, trigger render thread
		TickFinished();
	}
    
#if defined (TI_PLATFORM_IOS)
    void TEngine::TickIOS()
    {
        Tick();
    }
#endif

	void TEngine::Tick()
	{
		uint64 CurrentFrameTime = TTimer::GetCurrentTimeMillis();
		uint32 Delta = (uint32)(CurrentFrameTime - LastFrameTime);
		float  Dt = Delta * 0.001f;
		if (bFreezeTick)
		{
			Dt = 0.f;
			if (bStepNext)
			{
				Dt = Delta * 0.001f;
				bStepNext = false;
			}
		}

		GameTimeElapsed += Dt;

		BeginFrame();

		// update inputs
		Device->GetInput()->UpdateEvents(Dt);

		Scene->TickAllNodes(Dt);

		for (auto T : Tickers)
		{
			T->Tick(Dt);
		}

		// Remember last frame start time
		LastFrameTime = CurrentFrameTime;

		EndFrame();

		// Check if reach max frame interval time
		uint32 TimePast = (uint32)(TTimer::GetCurrentTimeMillis() - CurrentFrameTime);
		if (TimePast < FrameInterval)
		{
			TThread::ThreadSleepAccurate(double(FrameInterval - TimePast));
		}
	}

	void TEngine::TickFinished()
	{
		FRenderThread * RT = FRenderThread::Get();
		// Game thread goes 2 frames ahead of render thread, more frames take more memory.
		static const int32 GameThreadFramesAhead = 1;
		while (RT->GetTriggerNum() >= GameThreadFramesAhead)
		{
			TThread::ThreadSleep(1);
		}

		RT->TriggerRender();
	}

	void TEngine::AddTask(TTask * Task)
	{
		MainThreadTasks.PushBack(Task);
	}

	void TEngine::DoTasks()
	{
		const int32 MaxTaskInEachFrame = 32;
		TTask* Task;
		int32 TaskExecuted = MaxTaskInEachFrame;
		while (MainThreadTasks.GetSize() > 0)
		{
			MainThreadTasks.PopFront(Task);
			Task->Execute();

			// release task memory
			if (!Task->HasNextTask())
			{
				ti_delete Task;
				Task = nullptr;
			}

			--TaskExecuted;
			if (TaskExecuted < 0)
				break;
		}
	}
}
