/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FVTTaskThread.h"

namespace tix
{
	FVTTaskThread::FVTTaskThread()
		: TThread("VTTaskThread")
	{
	}

	FVTTaskThread::~FVTTaskThread()
	{
	}

	void FVTTaskThread::Run()
	{
		if (Buffers.size() == 0 && VTLoadTasks.size() == 0)
		{
			// Has nothing to do, wait for next loop
			TThread::ThreadSleep(10);
		}
		else
		{
			// Analysis buffer first, in order to get loading tasks
			if (Buffers.size() > 0)
			{
				AnalysisBuffer();
			}
			else if (VTLoadTasks.size() > 0)
			{
				// Do loading task
			}
		}
	}

	void FVTTaskThread::AddUVBuffer(TStreamPtr InBuffer)
	{
		BufferMutex.lock();
		Buffers.push_back(InBuffer);
		BufferMutex.unlock();
	}

	void FVTTaskThread::AnalysisBuffer()
	{
		TStreamPtr Buffer;
		BufferMutex.lock();
		Buffer = Buffers.front();
		Buffers.pop_front();
		BufferMutex.unlock();

		const FFloat4* Data = (const FFloat4*)Buffer->GetBuffer();
		const int32 DataCount = Buffer->GetLength() / sizeof(FFloat4);
	}
}