/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TResourceLoadingTask : public TTask
	{
	public:
		enum 
		{
			STEP_IO,
			STEP_PARSE,
			STEP_FINISHED,
		};
		TResourceLoadingTask(const TString& InResFilename, TResourceObjectPtr InResourceObject)
			: ResFilename(InResFilename)
			, LoadingStep(STEP_IO)
			, ResourceObject(InResourceObject)
		{}
		virtual ~TResourceLoadingTask() {}

		virtual void Execute() override;
		virtual bool HasNextTask() override
		{
			return LoadingStep != STEP_FINISHED;
		}

		int32 GetLoadingStep() const
		{
			return LoadingStep;
		}

	private:
		TString ResFilename;
		int32 LoadingStep;
		TResourceObjectPtr ResourceObject;
	};

	class TThreadIO;
	class TThreadLoading : public TTaskThread
	{
	public:
		static void CreateLoadingThread();
		static void DestroyLoadingThread();
		static TThreadLoading * Get();

		virtual void AddTask(TTask* Task) override;

		virtual void OnThreadStart() override;
		inline static bool IsLoadingThread()
		{
			return TThread::GetThreadId() == LoadingThreadId;
		}

	private:
		static TThreadLoading* LoadingThread;
		TThreadLoading();
		virtual ~TThreadLoading();

		virtual void Start();
		virtual void Stop();

	protected:

	protected:
		TThreadIO * IOThread;

		static TThreadId LoadingThreadId;
	};

	inline bool IsLoadingThread()
	{
		return TThreadLoading::IsLoadingThread();
	}
}
