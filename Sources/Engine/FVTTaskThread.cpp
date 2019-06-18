/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FVTTaskThread.h"
#include "FVTSystem.h"
#include "TTextureLoader.h"

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
				DoLoadingTask();
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

		FVTSystem * VTSystem = FVTSystem::Get();
		const FFloat4* DataPtr = (const FFloat4*)Buffer->GetBuffer();
		const int32 DataCount = Buffer->GetLength() / sizeof(FFloat4);

		for (int32 i = 0 ; i < DataCount ; ++ i)
		{
			const FFloat4& Data = DataPtr[i];
			if (Data.W > 0.f)
			{
				vector2di Position;
				Position.X = (int32)(Data.X * FVTSystem::VTSize);
				Position.Y = (int32)(Data.Y * FVTSystem::VTSize);

				FVTSystem::FPageInfo PageInfo = VTSystem->GetPageInfoByPosition(Position);
				if ((PageInfo.PageIndex & 0x80000000) == 0)
				{
					static int32 a = 0;
					//_LOG(Log, "%d, Need Page [%4d, %4d], %s\n", a++, Position.X, Position.Y, PageInfo.TextureName.c_str());
					// Not loaded, add to load queue
					if (VTLoadTasks.find(PageInfo.PageIndex) == VTLoadTasks.end())
					{
						// Not in loading queue, add to it
						VTTaskOrder.push_back(PageInfo.PageIndex);
						VTLoadTasks[PageInfo.PageIndex] = PageInfo;
					}
				}
			}
		}
	}

	void FVTTaskThread::DoLoadingTask()
	{
		// Pop a task
		uint32 TaskIndex = VTTaskOrder.front();
		VTTaskOrder.pop_front();
		TI_ASSERT(VTLoadTasks.find(TaskIndex) != VTLoadTasks.end());
		FVTSystem::FPageInfo Task = VTLoadTasks[TaskIndex];
		VTLoadTasks.erase(TaskIndex);

		// Mark this page as loaded
		FVTSystem::Get()->MarkPageAsLoaded(Task.PageIndex, true);
		TI_ASSERT((Task.PageIndex & 0x80000000) == 0);

		// Load texture region
		int32 StartX = Task.PageStart.X * FVTSystem::PPSize;
		int32 StartY = Task.PageStart.Y * FVTSystem::PPSize;
		TTexturePtr Tex = TTextureLoader::LoadTextureWithRegion(Task.TextureName, 0, StartX, StartY, StartX + FVTSystem::PPSize, StartY + FVTSystem::PPSize);
		static int32 a = 0;
		_LOG(Log, "%d, Load tex [%4d, %4d] : %s\n", a++, StartX, StartY, Task.TextureName.c_str());
		//TI_ASSERT(0);

	}
}