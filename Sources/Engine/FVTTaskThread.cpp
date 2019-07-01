/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FVTTaskThread.h"
#include "FVTSystem.h"
#include "TVTTextureLoader.h"

namespace tix
{
	FVTAnalysisThread::FVTAnalysisThread()
		: TThread("VTAnalysisThread")
		, AnalysisDone(false)
		, VTLoadingThread(nullptr)
	{
		PagesInThisFrame.reserve(64);

		// Start loading thread
		VTLoadingThread = ti_new FVTLoadingThread;
		VTLoadingThread->Start();
	}

	FVTAnalysisThread::~FVTAnalysisThread()
	{
		VTLoadingThread->Stop();
		ti_delete VTLoadingThread;
	}

	void FVTAnalysisThread::Stop()
	{
		if (Thread != nullptr)
		{
			{
				unique_lock<TMutex> CLock(TaskBeginMutex);
				IsRunning = false;
				TaskBeginCond.notify_one();
			}
			Thread->join();

			ti_delete Thread;
			Thread = nullptr;
		}
	}

	void FVTAnalysisThread::Run()
	{
		{
			// wait for begin signal
			unique_lock<TMutex> TaskLock(TaskBeginMutex);
			TaskBeginCond.wait(TaskLock);
		}
		
		{
			TI_ASSERT(Buffers.size() > 0);
			AnalysisBuffer();
			TI_TODO("Analysis time consume");
		}

		{
			// notify finished signal
			unique_lock<TMutex> CLock(TaskFinishedMutex);
			AnalysisDone = true;
			TaskFinishedCond.notify_one();
		}
	}

	void FVTAnalysisThread::WaitForAnalysisFinished()
	{
		unique_lock<TMutex> TaskLock(TaskFinishedMutex);
		TaskFinishedCond.wait(TaskLock, [this] {return AnalysisDone; });
	}

	void FVTAnalysisThread::AddUVBuffer(TStreamPtr InBuffer)
	{
		unique_lock<TMutex> CLock(TaskBeginMutex);
		AnalysisDone = false;
		TaskBeginCond.notify_one();

		BufferMutex.lock();
		Buffers.push_back(InBuffer);
		BufferMutex.unlock();
	}

	struct FSortTask
	{
		FSortTask(THMap<uint32, FVTSystem::FPageLoadInfo>& InTasks)
			: LoadTasks(InTasks)
		{}

		THMap<uint32, FVTSystem::FPageLoadInfo>& LoadTasks;

		bool operator()(uint32 A, uint32 B)
		{
			return LoadTasks[A] < LoadTasks[B];
		}
	};

	void FVTAnalysisThread::AnalysisBuffer()
	{
		TStreamPtr Buffer;
		BufferMutex.lock();
		Buffer = Buffers.front();
		Buffers.pop_front();
		BufferMutex.unlock();

		FVTSystem * VTSystem = FVTSystem::Get();
		const FFloat4* DataPtr = (const FFloat4*)Buffer->GetBuffer();
		const int32 DataCount = Buffer->GetLength() / sizeof(FFloat4);

		// Task execute order, point to VTLoadTasks, will be used in VTLoadingThread
		FVTTaskOrderList* VTTaskOrder = ti_new FVTTaskOrderList();

		// The actual load task, Key is Page Index, Value is Page Load Info, will be used in VTLoadingThread
		FVTLoadTaskMap* VTLoadTasks = ti_new FVTLoadTaskMap();

		PagesInThisFrame.clear();
		THMap<uint32, uint32> PagesAlreadyRecorded;
		for (int32 i = 0 ; i < DataCount ; ++ i)
		{
			const FFloat4& Data = DataPtr[i];
			if (Data.W > 0.f)
			{
				vector2di Position;
				Position.X = (int32)(Data.X * FVTSystem::VTSize);
				Position.Y = (int32)(Data.Y * FVTSystem::VTSize);

				FVTSystem::FPageLoadInfo PageLoadInfo;
				VTSystem->GetPageLoadInfoByPosition(Position, PageLoadInfo);

				FTexturePtr PhysicPage;
				if (CachedTextures.find(PageLoadInfo.PageIndex) == CachedTextures.end())
				{
					// Send a dummy texture
					PhysicPage = FRHI::Get()->CreateTexture();
					CachedTextures[PageLoadInfo.PageIndex] = PhysicPage;
					PageLoadInfo.TargetTexture = PhysicPage;

					// Not in cache, add a task to load it.
					TI_ASSERT(VTLoadTasks->find(PageLoadInfo.PageIndex) == VTLoadTasks->end());
					{
						// Not in loading queue, add to it
						VTTaskOrder->push_back(PageLoadInfo.PageIndex);
						(*VTLoadTasks)[PageLoadInfo.PageIndex] = PageLoadInfo;
					}
				}
				else
				{
					// Already in cache, send to render thread directly.
					PhysicPage = CachedTextures[PageLoadInfo.PageIndex];
				}

				if (PagesAlreadyRecorded.find(PageLoadInfo.PageIndex) == PagesAlreadyRecorded.end())
				{
					FVTSystem::FPhyPageInfo PhyPageInfo;
					PhyPageInfo.Texture = PhysicPage;
					PhyPageInfo.PageIndex = PageLoadInfo.PageIndex;
					PagesInThisFrame.push_back(PhyPageInfo);

					PagesAlreadyRecorded[PageLoadInfo.PageIndex] = 1;
				}
			}
			TI_TODO("Remove tasks in the queue if it is not in this frame.");
		}

		VTTaskOrder->sort(FSortTask(*VTLoadTasks));
	}

	void FVTAnalysisThread::OutputDebugTasks(FVTTaskOrderList& VTTaskOrder, FVTLoadTaskMap& VTLoadTasks)
	{
		for (auto& i : VTTaskOrder)
		{
			_LOG(Log, "[%4d, %4d] %s.\n", VTLoadTasks[i].PageStart.X, VTLoadTasks[i].PageStart.Y, VTLoadTasks[i].TextureName.c_str());
		}
	}

	//////////////////////////////////////////////////////////////////////////

	FVTLoadingThread::FVTLoadingThread()
		: TThread("VTLoadingThread")
	{
	}

	FVTLoadingThread::~FVTLoadingThread()
	{
	}

	void FVTLoadingThread::OnThreadStart()
	{
		TThread::OnThreadStart();
		TThread::IndicateVTLoadingThread();
	}

	void FVTLoadingThread::Run()
	{
		if (VTLoadTaskInfos.size() == 0 && VTLoadingTasks.size() == 0)
		{
			TThread::ThreadSleep(5);
		}
		if (VTLoadTaskInfos.size() > 0)
		{
			CollectTasks();
		}
		else if (VTLoadingTasks.size() > 0)
		{
			DoLoadingTasks();
		}
	}

	void FVTLoadingThread::AddLoadingTasks(FVTAnalysisThread::FVTTaskOrderList* VTTaskOrder, FVTAnalysisThread::FVTLoadTaskMap* InVTLoadTasks)
	{
		TaskMutex.lock();
		VTTaskOrders.push_back(VTTaskOrder);
		VTLoadTaskInfos.push_back(InVTLoadTasks);
		TaskMutex.unlock();
	}

	void FVTLoadingThread::CollectTasks()
	{
		TVector<FVTAnalysisThread::FVTTaskOrderList*> VTTaskOrdersCopy;
		TVector<FVTAnalysisThread::FVTLoadTaskMap*> VTLoadTaskInfosCopy;
		{
			TaskMutex.lock();
			VTTaskOrdersCopy = VTTaskOrders;
			VTLoadTaskInfosCopy = VTLoadTaskInfos;
			VTTaskOrders.clear();
			VTLoadTaskInfos.clear();
			TaskMutex.unlock();
		}

		const int32 Size = (int32)VTTaskOrdersCopy.size();
		FVTLoadingTask Task;
		for (int32 i = 0; i < Size; ++i)
		{
			FVTAnalysisThread::FVTTaskOrderList* TaskList = VTTaskOrdersCopy[i];
			FVTAnalysisThread::FVTLoadTaskMap* TaskMap = VTLoadTaskInfosCopy[i];
			for (auto& t : *TaskList)
			{
				FVTSystem::FPageLoadInfo& Info = (*TaskMap)[t];

				if (Task.Name != Info.TextureName)
				{
					if (!Task.Name.empty())
					{
						VTLoadingTasks.push_back(Task);
					}
					Task.Name = Info.TextureName;
				}

				Task.PageIndex.push_back(Info.PageIndex);
				Task.PageStart.push_back(Info.PageStart);
				Task.TextureResource.push_back(Info.TargetTexture);
			}
			VTLoadingTasks.push_back(Task);

			ti_delete TaskList;
			ti_delete TaskMap;
		}
		_LOG(Log, "%d VT Task collected.\n", VTLoadingTasks.size());
	}

	void FVTLoadingThread::DoLoadingTasks()
	{
		FVTLoadingTask Task = VTLoadingTasks.front();
		VTLoadingTasks.pop_front();

		const int32 Pages = (int32)Task.PageIndex.size();

		TVector<TTexturePtr> TextureLoaded;
		TextureLoaded.reserve(Pages);
		TVTTextureLoader Loader(Task.Name);
		
		for (int32 i = 0 ; i < Pages; ++ i)
		{
			// Load texture region
			int32 StartX = Task.PageStart[i].X * FVTSystem::PPSize;
			int32 StartY = Task.PageStart[i].Y * FVTSystem::PPSize;
			TTexturePtr Texture = Loader.LoadTextureWithRegion(0, StartX, StartY, StartX + FVTSystem::PPSize, StartY + FVTSystem::PPSize);
			TextureLoaded.push_back(Texture);
		}

		// Send to render thread to init render resource of this texture
		ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(FVTLoadingUpdateTexture,
			TVector<uint32>, PageIndex, Task.PageIndex,
			TVector<FTexturePtr>, TextureResource, Task.TextureResource,
			TVector<TTexturePtr>, TextureData, TextureLoaded,
			{
				FVTSystem::Get()->UpdateLoadedPages(PageIndex, TextureResource, TextureData);
			});
	}
}