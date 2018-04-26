/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TI_API TTask
	{
	public:
		TTask();
		virtual ~TTask();

		virtual void Execute() = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	class TI_API TTaskThread : public TThread
	{
	public:
		TTaskThread(const TString& Name);
		virtual ~TTaskThread();
		
		virtual void Run() override;
		virtual void Stop() override;

		void AddTask(TTask* Task);
		
	protected:
		virtual void DoTasks();

	protected:
		typedef TThreadSafeQueue<TTask*> TTaskQueue;
		TTaskQueue		Tasks;
		TTaskQueue		TaskFinished;

		TMutex			TaskMutex;
		TCond			TaskCond;
	};
}