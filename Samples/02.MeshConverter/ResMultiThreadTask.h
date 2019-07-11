/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "TThread.h"

namespace tix
{
	class TResMTTask
	{
	public:
		virtual void Exec() = 0;

	};

	////////////////////////////////////////////////////////////////////
	class TResMTTaskExecuter;

	class TResTaskThread : public TThread
	{
	public:
		TResTaskThread(const TString& Name, TResMTTaskExecuter * InExecuter, int32 Index);
		~TResTaskThread();

		virtual void Run() override;
		virtual void Stop() override;

		void AddTask(TResMTTask* Task)
		{
			Tasks.push_back(Task);
		}
		void StartExecute();

	private:
		TResMTTaskExecuter * Executer;
		int32 TaskIndex;

		TMutex TaskMutex;
		TCond TaskCond;
		TVector<TResMTTask*> Tasks;
	};

	////////////////////////////////////////////////////////////////////////

	class TResMTTaskExecuter
	{
	public:
		static TResMTTaskExecuter * Get();
		static TResMTTaskExecuter * Create();
		static void Destroy();

		int32 GetMaxThreadCount() const
		{
			return MaxThreadCount;
		}

		void AddTask(TResMTTask * Task);
		void StartTasks();
		void WaitUntilFinished();

		enum 
		{
			STATE_NONE,
			STATE_EXECUTING,
		};
	protected:
		TResMTTaskExecuter();
		~TResMTTaskExecuter();
		void InitThreads();
		void NotifyThreadFinished(int32 Index);

	protected:
		int32 MaxThreadCount;
		TVector<TResTaskThread*> Threads;

		int32 TaskThreadIndex;
		int32 ExecuteState;
		int32 RunningThreads;
		TMutex RunningThreadsMutex;
		THMap<int32, int32> UsedThreads;
		friend class TResTaskThread;
	};
}