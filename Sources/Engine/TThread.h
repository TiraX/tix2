/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TI_API TThread
	{
	public:
		TThread(const TString& Name);
		virtual ~TThread();

		static TThreadId GetThreadId();
		static void	ThreadSleep(uint32 milliseconds);

		static void IndicateGameThread();
		static void IndicateRenderThread();

		static bool IsGameThread();
		static bool IsRenderThread();
		
		virtual void Start();
		virtual void Stop();

		virtual void Run() = 0;
		virtual void OnThreadStart();
		virtual void OnThreadEnd() {};
		
		virtual bool ThreadRunning()
		{
			return	IsRunning;
		}
	private:
		static void* ThreadExecute(void* param);
		void SetThreadName();
	protected:
		bool IsRunning;
		thread * Thread;
		TThreadId ThreadId;

	private:
		TString	ThreadName;

		static TThreadId GameThreadId;
		static TThreadId RenderThreadId;
	};

	inline bool IsGameThread()
	{
		return TThread::IsGameThread();
	}

	inline bool IsRenderThread()
	{
		return TThread::IsRenderThread();
	}
}