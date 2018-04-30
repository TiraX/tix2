/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TTaskThread.h"

namespace tix
{
	TTask::TTask()
	{
	}

	TTask::~TTask()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	TTaskThread::TTaskThread(const TString& Name)
		: TThread(Name)
	{
	}

	TTaskThread::~TTaskThread()
	{
	}

	void TTaskThread::Run()
	{
		unique_lock<TMutex> TaskLock(TaskMutex);
		TaskCond.wait(TaskLock);

		DoTasks();
	}

	void TTaskThread::Stop()
	{
		if (Thread != nullptr)
		{
			{
				unique_lock<TMutex> CLock(TaskMutex);
				IsRunning = false;
				TaskCond.notify_one();
			}
			Thread->join();

			ti_delete Thread;
			Thread = nullptr;
		}
	}

	void TTaskThread::DoTasks()
	{
		TI_ASSERT(GetThreadId() == ThreadId);
		TTask* Task;
		while (Tasks.GetSize() > 0)
		{
			Tasks.PopFront(Task);
			Task->Execute();

			// release task memory
			ti_delete Task;
			Task = nullptr;
		}
	}

	void TTaskThread::AddTask(TTask* Task)
	{
		TI_ASSERT(IsGameThread());

		unique_lock<TMutex> CLock(TaskMutex);
		Tasks.PushBack(Task);
		TaskCond.notify_one();
	}
}