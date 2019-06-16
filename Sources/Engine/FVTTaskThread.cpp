/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FVTTaskThread.h"
#include "FVTSystem.h"

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
				if ((PageInfo.RegionData & 0x80000000) != 0)
				{
					VTLoadTasks.push_back(PageInfo);
				}
			}
		}
	}

	void FVTTaskThread::DoLoadingTask()
	{
		FVTSystem::FPageInfo Task = VTLoadTasks.front();
		VTLoadTasks.pop_front();

		TAssetFilePtr AssetFile = ti_new TAssetFile;
		if (AssetFile->Load(Task.TextureName))
		{
			int32 StartX = Task.PageStart.X * FVTSystem::PPSize;
			int32 StartY = Task.PageStart.Y * FVTSystem::PPSize;
			TTexturePtr Tex = AssetFile->CreateTextureWithRegion(0, StartX, StartY, StartX + FVTSystem::PPSize, StartY + FVTSystem::PPSize);

		}
	}
}