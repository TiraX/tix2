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

		TI_API void WaitForAnalysisFinished();

		const TVector<FVTSystem::FPhyPageInfo>& GetPhysicPages() const
		{
			return PagesInThisFrame;
		}
	protected:
		void AnalysisBuffer();
		void OutputDebugTasks(FVTTaskOrderList& VTTaskOrder, FVTLoadTaskMap& VTLoadTasks);

	private:
		// Analysis thread begin TCond signal
		TMutex TaskBeginMutex;
		TCond TaskBeginCond;

		// Analysis thread finished TCond signal
		TMutex TaskFinishedMutex;
		TCond TaskFinishedCond;

		// VT UV Buffers of 1/8 x 1/8 screen, send from Render Thread
		TList<TStreamPtr> Buffers;
		TMutex BufferMutex;

		// Cached textures, Key is Page Index, Value is FTexture
		THMap<uint32, FTexturePtr> CachedTextures;

		// Physic pages need in this frame
		TVector<FVTSystem::FPhyPageInfo> PagesInThisFrame;

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
			TVector<FTexturePtr> TextureResource;
		};
		TList<FVTLoadingTask> VTLoadingTasks;
	};
}