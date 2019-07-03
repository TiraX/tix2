/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FVTLoadingThread;
	class FVTAnalysisThread: public TThread
	{
	public:
		typedef TList<uint32> FVTTaskOrderList;
		typedef THMap<uint32, FVTSystem::FPageLoadInfo> FVTLoadTaskMap;

		FVTAnalysisThread();
		virtual ~FVTAnalysisThread();

		TI_API void AddUVBuffer(TStreamPtr Buffer);
		virtual void Run() override;
		virtual void Stop() override;

	protected:
		void AnalysisBuffer();
		int32 FindAvailbleLocation(const THMap<uint32, uint32>& UsedLocations);
		void OutputDebugTasks(FVTTaskOrderList& VTTaskOrder, FVTLoadTaskMap& VTLoadTasks);

	private:
		// Analysis thread begin TCond signal
		TMutex TaskBeginMutex;
		TCond TaskBeginCond;

		// VT UV Buffers of 1/8 x 1/8 screen, send from Render Thread
		TList<TStreamPtr> Buffers;
		TMutex BufferMutex;

		// Physic texture page array, stores the page index in virtual texture
		TVector<uint32> PhysicPageTextures;

		// Physic page's location in PhysicPageTextures
		// Key is the page index in virtual texture, Value is the index in Physic pages array
		THMap<uint32, uint32> PhysicPagesMap;

		// Next availble location;
		int32 SearchedLocation;

		FVTLoadingThread * VTLoadingThread;
	};

	//////////////////////////////////////////////////////////////////////////

	class FVTLoadingThread : public TThread
	{
	public:
		FVTLoadingThread();
		~FVTLoadingThread();

		virtual void OnThreadStart() override;
		virtual void Run() override;

		void AddLoadingTasks(FVTAnalysisThread::FVTTaskOrderList* VTTaskOrder, FVTAnalysisThread::FVTLoadTaskMap* InVTLoadTasks);

	private:
		void CollectTasks();
		void DoLoadingTasks();

	private:
		TVector<FVTAnalysisThread::FVTTaskOrderList*> VTTaskOrders;
		TVector<FVTAnalysisThread::FVTLoadTaskMap*> VTLoadTaskInfos;
		TMutex TaskMutex;

		struct FVTLoadingTask
		{
			TString Name;
			TVector<vector2du16> PageStart;
			TVector<uint32> PageIndex;
			TVector<uint32> AtlasLocation;
		};
		TList<FVTLoadingTask> VTLoadingTasks;
	};
}