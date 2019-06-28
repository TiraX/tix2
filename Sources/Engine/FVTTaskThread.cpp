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

	void FVTTaskThread::OnThreadStart()
	{
		TThread::OnThreadStart();
		TThread::IndicateVTTaskThread();
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

	struct FSortTask
	{
		FSortTask(THMap<uint32, FVTSystem::FPageInfo>& InTasks)
			: LoadTasks(InTasks)
		{}

		THMap<uint32, FVTSystem::FPageInfo>& LoadTasks;

		bool operator()(uint32 A, uint32 B)
		{
			return LoadTasks[A] < LoadTasks[B];
		}
	};

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

				FVTSystem::FPageInfo PageInfo;
				VTSystem->GetPageInfoByPosition(Position, PageInfo);

				if (CachedTextures.find(PageInfo.PageIndex) == CachedTextures.end())
				{
					// Not in cache, add a task to load it.
					if (VTLoadTasks.find(PageInfo.PageIndex) == VTLoadTasks.end())
					{
						// Not in loading queue, add to it
						VTTaskOrder.push_back(PageInfo.PageIndex);
						VTLoadTasks[PageInfo.PageIndex] = PageInfo;
					}
				}
				else
				{
					// Already in cache, send to render thread directly.
					TI_ASSERT(0);
				}
			}
			TI_TODO("Remove tasks in the queue if it is not in this frame.");
		}
		//OutputDebugTasks();
		VTTaskOrder.sort(FSortTask(VTLoadTasks));
		//_LOG(Log, "------------------------\n");
		//OutputDebugTasks();
		//_LOG(Log, "------------------------\n");
	}

	void FVTTaskThread::OutputDebugTasks()
	{
		for (auto& i : VTTaskOrder)
		{
			_LOG(Log, "[%4d, %4d] %s.\n", VTLoadTasks[i].PageStart.X, VTLoadTasks[i].PageStart.Y, VTLoadTasks[i].TextureName.c_str());
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

		FVTSystem* VTSystem = FVTSystem::Get();

		// Load texture region
		int32 StartX = Task.PageStart.X * FVTSystem::PPSize;
		int32 StartY = Task.PageStart.Y * FVTSystem::PPSize;
		TTexturePtr Tex = TTextureLoader::LoadTextureWithRegion(Task.TextureName, 0, StartX, StartY, StartX + FVTSystem::PPSize, StartY + FVTSystem::PPSize);
		Tex->TextureResource = FRHI::Get()->CreateTexture(Tex->GetDesc());

		{
			static int32 a = 0;
			_LOG(Log, "%d, Load tex [%4d, %4d] : %s\n", a++, StartX, StartY, Task.TextureName.c_str());
		}

		// Send to render thread to init render resource of this texture
		FVTSystem::FPhysicPageTexture PhysicPageTexture;
		PhysicPageTexture.Texture = Tex;
		PhysicPageTexture.PhysicPagePosition = Task.PhysicPage;
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(FVTTaskInitPhysicTexture,
			FVTSystem::FPhysicPageTexture, PhysicPageTexture, PhysicPageTexture,
			{
				FVTSystem::Get()->UpdatePhysicPage_RenderThread(PhysicPageTexture);
			});
	}
}