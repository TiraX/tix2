/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRenderThread.h"
#include "FRenderer.h"
#include "FRHI.h"

namespace tix
{
	FRenderThread* FRenderThread::RenderThread = nullptr;
	bool FRenderThread::Inited = false;

	void FRenderThread::CreateRenderThread()
	{
		TI_ASSERT(RenderThread == nullptr);
		RenderThread = ti_new FRenderThread;
		RenderThread->Start();
	}

	void FRenderThread::DestroyRenderThread()
	{
		TI_ASSERT(RenderThread != nullptr);
		RenderThread->Stop();
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
		Renderers.push_back(Renderer);
	}

	void FRenderThread::CreateRenderComponents()
	{
		// Create RHI to submit commands to GPU
		FRHI::CreateRHI(ERHI_DX12);
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
		
		RHI->BeginFrame();
		// Do render thread tasks
		DoRenderTasks();

		// Go through each renderer
		for (auto Renderer : Renderers)
		{
			Renderer->Render(RHI, RenderScene);
		}
		RHI->EndFrame();
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
		if (Thread != nullptr)
		{
			TriggerRenderAndStop();
			Thread->join();

			ti_delete Thread;
			Thread = nullptr;
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

			// release task memory
			ti_delete Task;
			Task = nullptr;
		}

		// Move to next
		RenderFrameIndex = (RenderFrameIndex + 1) % FRHIConfig::FrameBufferNum;
	}

	void FRenderThread::TriggerRender()
	{
		unique_lock<TMutex> RenderLock(RenderMutex);
		// Add Trigger Number
		++TriggerNum;
		// Frame index move to next frame. Close current frame data
		PreFrameIndex = (PreFrameIndex + 1) % FRHIConfig::FrameBufferNum;
		RenderCond.notify_one();
	}

	void FRenderThread::TriggerRenderAndStop()
	{
		unique_lock<TMutex> RenderLock(RenderMutex);
		// Add Trigger Number
		++TriggerNum;
		// Frame index move to next frame. Close current frame data
		PreFrameIndex = (PreFrameIndex + 1) % FRHIConfig::FrameBufferNum;
		IsRunning = false;
		RenderCond.notify_one();
	}

	void FRenderThread::WaitForRenderSignal()
	{
		unique_lock<TMutex> RenderLock(RenderMutex);
		--TriggerNum;
		RenderCond.wait(RenderLock);
	}

	void FRenderThread::AddTaskToFrame(TTask* Task)
	{
		TI_ASSERT(IsGameThread());
		RenderFrames[PreFrameIndex].FrameTasks.PushBack(Task);
	}
}