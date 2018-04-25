/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TI_API TMutex
	{
	public:
		TMutex();
		~TMutex();

		void		Lock();
		void		Unlock();

	protected:
		mutex		Mutex;
	};

	//////////////////////////////////////////////////////////////////////////

	//class TI_API TCond
	//{
	//public:
	//	TCond();
	//	~TCond();

	//	void		Wait();
	//	void		Signal();

	//protected:
	//	mutex		Mutex;
	//	condition_variable Cond;
	//};

	//////////////////////////////////////////////////////////////////////////

	class TI_API TThread
	{
	public:
		TThread(const TString& Name);
		virtual ~TThread();
		
		static void	ThreadSleep(uint32 milliseconds);

		static void* ThreadExecute(void* param);

		virtual void Start();
		virtual void Stop();

		virtual void Run() = 0;
		virtual void OnThreadStart() {};
		virtual void OnThreadEnd() {};
		
		virtual bool ThreadRunning()
		{
			return	IsRunning;
		}
	private:
		void SetThreadName();
	protected:
		bool IsRunning;
		thread * Thread;

	private:
		TString	ThreadName;
	};
}