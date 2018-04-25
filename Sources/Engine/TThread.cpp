/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TThread.h"

namespace tix
{
	TMutex::TMutex()
	{
	}

	TMutex::~TMutex()
	{
	}

	void TMutex::Lock()
	{
		Mutex.lock();
	}

	void TMutex::Unlock()
	{
		Mutex.unlock();
	}

	//////////////////////////////////////////////////////////////////////////

	//TCond::TCond()
	//{
	//}

	//TCond::~TCond()
	//{
	//}
	//
	//void TCond::Wait()
	//{
	//	unique_lock<mutex> CLock(Mutex);
	//	Cond.wait(CLock);
	//}

	//void TCond::Signal()
	//{
	//	unique_lock<mutex> CLock(Mutex);
	//	Cond.notify_one();
	//}
	//////////////////////////////////////////////////////////////////////////
	TThread::TThread(const TString& Name)
		: IsRunning(false)
		, Thread(nullptr)
		, ThreadName(Name)
	{
	}

	TThread::~TThread()
	{
		Stop();
	}

	void TThread::ThreadSleep(uint32 milliseconds)
	{
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	}

	void* TThread::ThreadExecute(void* param)
	{
		TThread* t = (TThread*)param;
		t->SetThreadName();

		t->OnThreadStart();

		while (t->IsRunning)
		{
			t->Run();
		}

		t->OnThreadEnd();

		return 0;
	}

	void TThread::Start()
	{
		if (IsRunning) 
		{
			return;
		}
	
		IsRunning = true;

		TI_ASSERT(Thread == nullptr);
		Thread = ti_new thread(TThread::ThreadExecute, this);
	}

	void TThread::Stop()
	{
		if (Thread != nullptr)
		{
			IsRunning = false;
			Thread->join();

			ti_delete Thread;
			Thread = nullptr;
		}
	}

#ifdef TI_PLATFORM_WIN32
	const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)  
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.  
		LPCSTR szName; // Pointer to name (in user addr space).  
		DWORD dwThreadID; // Thread ID (-1=caller thread).  
		DWORD dwFlags; // Reserved for future use, must be zero.  
	} THREADNAME_INFO;
#pragma pack(pop)  
#endif

	void TThread::SetThreadName()
	{
#ifdef TI_PLATFORM_WIN32
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = ThreadName.c_str();
		info.dwThreadID = GetCurrentThreadId();
		info.dwFlags = 0;
#pragma warning(push)  
#pragma warning(disable: 6320 6322)  
		__try {
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
		}
#pragma warning(pop)  
#else
#error("TThread::SetThreadName() Not implement for other platform yet.")
#endif
	}
}