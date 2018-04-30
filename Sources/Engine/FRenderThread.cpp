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
		, TriggerNum(0)
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

	void FRenderThread::Run()
	{
		// Waiting for Game thread tick
		WaitForRenderSignal();

		// Do render thread tasks
		DoTasks();

		// Go through each renderer
		for (auto Renderer : Renderers)
		{
			Renderer->Render(RHI);
		}
	}

	void FRenderThread::OnThreadStart()
	{
		TTaskThread::OnThreadStart();
		TThread::IndicateRenderThread();

		CreateRenderComponents();
	}

	void FRenderThread::OnThreadEnd()
	{
		TTaskThread::OnThreadEnd();

		DestroyRenderComponents();
	}

	void FRenderThread::TriggerRender()
	{
		unique_lock<TMutex> RenderLock(RenderMutex);
		++TriggerNum;
		RenderCond.notify_one();
	}

	void FRenderThread::WaitForRenderSignal()
	{
		unique_lock<TMutex> RenderLock(RenderMutex);
		--TriggerNum;
		RenderCond.wait(RenderLock);
	}
}