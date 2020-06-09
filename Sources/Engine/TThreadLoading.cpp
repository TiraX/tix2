/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TThreadLoading.h"
#include "TThreadIO.h"

namespace tix
{
	void TLoadingTask::Execute()
	{
		if (LoadingStep == STEP_IO)
		{
			TI_ASSERT(IsIOThread());
			ExecuteInIOThread();
			LoadingStep = STEP_LOADING;
			// Forward to loading thread
			TThreadLoading::Get()->AddTask(this);
		}
		else if (LoadingStep == STEP_LOADING)
		{
			TI_ASSERT(IsLoadingThread());
			ExecuteInLoadingThread();
			LoadingStep = STEP_BACK_TO_MAINTHREAD;
			// Return to main thread
			TEngine::Get()->AddTask(this);
		}
		else if (LoadingStep == STEP_BACK_TO_MAINTHREAD)
		{
			TI_ASSERT(IsGameThread());
			ExecuteInMainThread();
			LoadingStep = STEP_FINISHED;
		}
	}

	///////////////////////////////////////////////////////////////

	TThreadLoading* TThreadLoading::LoadingThread = nullptr;
	TThreadId TThreadLoading::LoadingThreadId;

	void TThreadLoading::CreateLoadingThread()
	{
		TI_ASSERT(LoadingThread == nullptr);
		LoadingThread = ti_new TThreadLoading;
		LoadingThread->Start();
	}

	void TThreadLoading::DestroyLoadingThread()
	{
		TI_ASSERT(LoadingThread != nullptr);
		LoadingThread->Stop();
		ti_delete LoadingThread;
		LoadingThread = nullptr;
	}

	TThreadLoading * TThreadLoading::Get()
	{
		return LoadingThread;
	}

	TThreadLoading::TThreadLoading()
		: TTaskThread("LoadingThread")
		, IOThread(nullptr)
	{
		// Start a IO thread
		IOThread = ti_new TThreadIO;
	}

	TThreadLoading::~TThreadLoading()
	{
		ti_delete IOThread;
		IOThread = nullptr;
	}

	void TThreadLoading::Start()
	{
		// Start loading thread self
		TTaskThread::Start();

		// Start IO Thread
		IOThread->Start();
	}

	void TThreadLoading::Stop()
	{
		// Stop IO Thread
		IOThread->Stop();

		// Stop loading thread self
		TTaskThread::Stop();
	}

	void TThreadLoading::AddTask(TTask* Task)
	{
		TLoadingTask* LoadingTask = static_cast<TLoadingTask*>(Task);
		if (LoadingTask->GetLoadingStep() == TLoadingTask::STEP_IO)
		{
			// Send Task to IO Thread first
			IOThread->AddTask(Task);
		}
		else if (LoadingTask->GetLoadingStep() == TLoadingTask::STEP_LOADING)
		{
			TTaskThread::AddTask(Task);
		}
		else
		{
			// Should not have other state
			TI_ASSERT(0);
		}
	}

	void TThreadLoading::OnThreadStart()
	{
		TTaskThread::OnThreadStart();

		LoadingThreadId = ThreadId;
	}
}
