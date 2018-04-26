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
	FRenderThread::FRenderThread()
		: TTaskThread("RenderThread")
		, RHI(nullptr)
	{
	}

	FRenderThread::~FRenderThread()
	{
	}

	void FRenderThread::AddRenderer(FRenderer* Renderer)
	{
		TI_TODO("Check is in render thread.");
		Renderers.push_back(Renderer);
	}

	void FRenderThread::CreateRenderComponents()
	{
		// Create RHI to submit commands to GPU
		RHI = FRHI::CreateRHI(ERHI_DX12);
	}

	void FRenderThread::DestroyRenderComponents()
	{
		// Release all renderers
		for (auto Renderer : Renderers)
		{
			ti_delete Renderer;
		}
		Renderers.clear();

		// Release RHI
		SAFE_DELETE(RHI);
	}

	static const uint64 FrameInterval = 33;
	void FRenderThread::Run()
	{
		uint64 CurrentFrameTime = TTimer::GetCurrentTimeMillis();
		uint32 Delta = (uint32)(CurrentFrameTime - LastFrameTime);

		// Do render thread tasks
		DoTasks();

		// Go through each renderer
		for (auto Renderer : Renderers)
		{
			Renderer->Render(RHI);
		}

		LastFrameTime = CurrentFrameTime;
		if (Delta < FrameInterval)
		{
			TThread::ThreadSleep(FrameInterval - Delta);
		}
	}

	void FRenderThread::OnThreadStart()
	{
		CreateRenderComponents();

		LastFrameTime = TTimer::GetCurrentTimeMillis();
	}

	void FRenderThread::OnThreadEnd()
	{
		DestroyRenderComponents();
	}
}