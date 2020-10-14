/*
	Copyright (C) 2012
	By Zhao Shuai tirax.cn@gmail.com 2012.9.04
*/

#include "stdafx.h"
#include "ResMultiThreadTask.h"

namespace tix
{
	TResTaskThread::TResTaskThread(const TString& Name, TResMTTaskExecuter * InExecuter, int32 Index)
		: TThread(Name)
		, Executer(InExecuter)
		, TaskIndex(Index)
	{
	}

	TResTaskThread::~TResTaskThread()
	{
	}

	void TResTaskThread::Run()
	{
		unique_lock<TMutex> TaskLock(TaskMutex);
		TaskCond.wait(TaskLock);

		if (Tasks.size() > 0)
		{
			for (auto T : Tasks)
			{
				T->Exec();
			}
			Tasks.clear();

			Executer->NotifyThreadFinished(TaskIndex);
		}
	}

	void TResTaskThread::Stop()
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

	void TResTaskThread::StartExecute()
	{
		TaskCond.notify_one();
	}

	////////////////////////////////////////////////////////////////////////
	static TResMTTaskExecuter * s_executer = nullptr;
	TResMTTaskExecuter * TResMTTaskExecuter::Get()
	{
		return s_executer;
	}

	TResMTTaskExecuter * TResMTTaskExecuter::Create()
	{
		if (s_executer == nullptr)
		{
			s_executer = ti_new TResMTTaskExecuter;
		}
		return s_executer;
	}

	void TResMTTaskExecuter::Destroy()
	{
		if (s_executer)
		{
			ti_delete s_executer;
			s_executer = nullptr;
		}
	}

	TResMTTaskExecuter::TResMTTaskExecuter()
		: TaskThreadIndex(0)
		, ExecuteState(STATE_NONE)
		, RunningThreads(0)
	{
		MaxThreadCount = TPlatformUtils::GetProcessorCount();
		InitThreads();
	}

	TResMTTaskExecuter::~TResMTTaskExecuter()
	{
		for (int32 i = 0; i < MaxThreadCount; ++i)
		{
			Threads[i]->Stop();
		}
	}

	void TResMTTaskExecuter::InitThreads()
	{
		TI_ASSERT(MaxThreadCount > 0);
		Threads.resize(MaxThreadCount);

		int8 ThreadName[128];
		for (int32 i = 0 ; i < MaxThreadCount ; ++ i)
		{
			sprintf(ThreadName, "ResTask%d", i);
			Threads[i] = ti_new TResTaskThread(ThreadName, this, i);
			Threads[i]->Start();
		}
	}

	void TResMTTaskExecuter::AddTask(TResMTTask * Task)
	{
		TI_ASSERT(ExecuteState == STATE_NONE);
		Threads[TaskThreadIndex]->AddTask(Task);
		UsedThreads[TaskThreadIndex] = 1;
		TaskThreadIndex = (TaskThreadIndex + 1) % MaxThreadCount;
	}

	void TResMTTaskExecuter::StartTasks()
	{
		ExecuteState = STATE_EXECUTING;
		TaskThreadIndex = 0;
		RunningThreadsMutex.lock();
		int32 TotalThreads = (int32)UsedThreads.size();
		RunningThreads = TotalThreads;
		RunningThreadsMutex.unlock();
		for (int32 i = 0; i < TotalThreads; ++i)
		{
			Threads[i]->StartExecute();
		}
	}

	void TResMTTaskExecuter::NotifyThreadFinished(int32 Index)
	{
		RunningThreadsMutex.lock();
		-- RunningThreads;
		TI_ASSERT(RunningThreads >= 0);
		RunningThreadsMutex.unlock();
	}

	void TResMTTaskExecuter::WaitUntilFinished()
	{
		while (RunningThreads > 0)
		{
			TThread::ThreadSleep(10);
		}
		ExecuteState = STATE_NONE;
		TaskThreadIndex = 0;
		UsedThreads.clear();
	}
}
