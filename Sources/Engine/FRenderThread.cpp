/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRenderThread.h"
#include "FRenderer.h"
#include "FRHI.h"

namespace tix
{
#if COMPILE_WITH_RHI_METAL
#   define AUTORELEASE_POOL_START @autoreleasepool {
#   define AUTORELEASE_POOL_END }
#else
#   define AUTORELEASE_POOL_START
#   define AUTORELEASE_POOL_END
#endif
	FRenderThread* FRenderThread::RenderThread = nullptr;
	bool FRenderThread::Inited = false;
	bool FRenderThread::ThreadEnabled = true;

	void FRenderThread::CreateRenderThread(bool ForceDisableThread)
	{
		FRenderThread::ThreadEnabled = !ForceDisableThread;
		TI_ASSERT(RenderThread == nullptr);
		RenderThread = ti_new FRenderThread;
		if (FRenderThread::ThreadEnabled)
		{
			RenderThread->Start();
		}
		else
		{
			RenderThread->OnThreadStart();
		}
	}

	void FRenderThread::DestroyRenderThread()
	{
		TI_ASSERT(RenderThread != nullptr);
		if (FRenderThread::ThreadEnabled)
		{
			RenderThread->Stop();
		}
		else
		{
			RenderThread->OnThreadEnd();
		}
		ti_delete RenderThread;
		RenderThread = nullptr;
	}

	FRenderThread* FRenderThread::Get()
	{
		return RenderThread;
	}

	bool FRenderThread::IsInited()
	{
		return Inited;
	}

	FRenderThread::FRenderThread()
		: TThread("RenderThread")
		, RHI(nullptr)
		, RenderScene(nullptr)
		, TriggerNum(0)
		, PreFrameIndex(0)
		, RenderFrameIndex(0)
	{
	}

	FRenderThread::~FRenderThread()
	{
	}

	void FRenderThread::AddRenderer(FRenderer* Renderer)
	{
		TI_ASSERT(IsRenderThread());
		Renderer->InitInRenderThread();
		Renderers.push_back(Renderer);
	}

	void FRenderThread::CreateRenderComponents()
	{
		// Create RHI to submit commands to GPU
		FRHI::CreateRHI();
		RHI = FRHI::Get();

		// Create render scene
		RenderScene = ti_new FScene;
	}

	void FRenderThread::DestroyRenderComponents()
	{
		// Finish all in-flight rendering
		RHI->WaitingForGpu();

		// Finish all un-finished tasks
		for (int32 i = 0; i < FRHIConfig::FrameBufferNum; i++)
		{
			DoRenderTasks();
		}

		// Release all renderers
		for (auto Renderer : Renderers)
		{
			ti_delete Renderer;
		}
		Renderers.clear();

		SAFE_DELETE(RenderScene);

		// Release RHI
		FRHI::ReleaseRHI();
	}

	void FRenderThread::Run()
	{
		// Waiting for Game thread tick
		WaitForRenderSignal();
        
		AUTORELEASE_POOL_START
        
		RHI->BeginFrame();
		// Do render thread tasks
		DoRenderTasks();

		// Go through each renderer
		for (auto Renderer : Renderers)
		{
			Renderer->Render(RHI, RenderScene);
		}
		RHI->EndFrame();
        
        AUTORELEASE_POOL_END
	}

	void FRenderThread::OnThreadStart()
	{
		TThread::OnThreadStart();
		TThread::IndicateRenderThread();

		CreateRenderComponents();

		Inited = true;
	}

	void FRenderThread::OnThreadEnd()
	{
		TThread::OnThreadEnd();

		DestroyRenderComponents();
	}

	void FRenderThread::Stop()
	{
		if (FRenderThread::ThreadEnabled)
		{
			if (Thread != nullptr)
			{
				TriggerRenderAndStop();
				Thread->join();

				ti_delete Thread;
				Thread = nullptr;
			}
		}
	}

	void FRenderThread::DoRenderTasks()
	{
		TI_ASSERT(IsRenderThread());
		TTask* Task;
		auto& Tasks = RenderFrames[RenderFrameIndex].FrameTasks;
		while (Tasks.GetSize() > 0)
		{
			Tasks.PopFront(Task);
			Task->Execute();
#if TIX_DEBUG_RENDER_TASK_NAME
			_LOG(Log, "%d - Do RT Task: %s\n", RenderFrameIndex, Task->GetTaskName().c_str());
#endif

			// release task memory
			if (!Task->HasNextTask())
			{
				ti_delete Task;
				Task = nullptr;
			}
		}

		// Move to next
		RenderFrameIndex = (RenderFrameIndex + 1) % FRHIConfig::FrameBufferNum;
	}

	void FRenderThread::TriggerRender()
	{
		if (FRenderThread::ThreadEnabled)
		{
			// Send a signal to trigger Run() in a single thread
			unique_lock<TMutex> RenderLock(RenderMutex);
			// Add Trigger Number
			++TriggerNum;
			// Frame index move to next frame. Close current frame data
			PreFrameIndex = (PreFrameIndex + 1) % FRHIConfig::FrameBufferNum;
			RenderCond.notify_one();
		}
		else
		{
			// Call Run() in this thread
			Run();
		}
	}

	void FRenderThread::TriggerRenderAndStop()
	{
		if (FRenderThread::ThreadEnabled)
		{
			unique_lock<TMutex> RenderLock(RenderMutex);
			// Add Trigger Number
			++TriggerNum;
			// Frame index move to next frame. Close current frame data
			PreFrameIndex = (PreFrameIndex + 1) % FRHIConfig::FrameBufferNum;
			IsRunning = false;
			RenderCond.notify_one();
		}
	}

	void FRenderThread::WaitForRenderSignal()
	{
		if (FRenderThread::ThreadEnabled)
		{
			unique_lock<TMutex> RenderLock(RenderMutex);
			--TriggerNum;
			RenderCond.wait(RenderLock);
		}
	}

	void FRenderThread::AddTaskToFrame(TTask* Task)
	{
		TI_ASSERT(IsGameThread());
		TI_ASSERT(RenderFrames[PreFrameIndex].FrameTasks.GetSize() + 1 < RenderFrames[PreFrameIndex].FrameTasks.GetContainerSize());
		RenderFrames[PreFrameIndex].FrameTasks.PushBack(Task);
	}
}
