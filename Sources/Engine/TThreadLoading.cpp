/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TThreadLoading.h"
#include "TThreadIO.h"

namespace tix
{
	void TResourceLoadingTask::Execute()
	{
		if (LoadingStep == STEP_IO)
		{
			_LOG(Log, "Doing IO.\n");
			// Read file content to FileBuffer
			TI_ASSERT(IsIOThread());
			ResourceObject->SourceFile = ti_new TResourceFile;
			ResourceObject->SourceFile->ReadFile(ResFilename);
			LoadingStep = STEP_PARSE;
			// Forward to loading thread
			TThreadLoading::Get()->AddTask(this);
		}
		else if (LoadingStep == STEP_PARSE)
		{
			_LOG(Log, "Doing Parse.\n");
			// Parse the buffer
			TI_ASSERT(IsLoadingThread());
			TI_ASSERT(ResourceObject->SourceFile->Filebuffer != nullptr);
			ResourceObject->SourceFile->ParseFile();
			ResourceObject->Resource = ResourceObject->SourceFile->CreateResource();
			LoadingStep = STEP_BACK_TO_MAINTHREAD;
			TI_ASSERT(ResourceObject->SourceFile->referenceCount() == 1);
			ResourceObject->SourceFile = nullptr;
			// Return to main thread
			TEngine::Get()->AddTask(this);
		}
		else if (LoadingStep == STEP_BACK_TO_MAINTHREAD)
		{
			TI_ASSERT(IsGameThread());
			// Init render thread resource
			ResourceObject->Resource->InitRenderThreadResource();

			LoadingStep = STEP_FINISHED;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	TThreadLoading* TThreadLoading::LoadingThread = nullptr;
	TThreadId TThreadLoading::LoadingThreadId;

	void TThreadLoading::CreateLoadingThread()
	{
		TI_ASSERT(LoadingThread == nullptr);
		LoadingThread = ti_new TThreadLoading;
		LoadingThread->Start();
	}

	void TThreadLoading::DestroyLoadingThread()
	{
		TI_ASSERT(LoadingThread != nullptr);
		LoadingThread->Stop();
		ti_delete LoadingThread;
		LoadingThread = nullptr;
	}

	TThreadLoading * TThreadLoading::Get()
	{
		return LoadingThread;
	}

	TThreadLoading::TThreadLoading()
		: TTaskThread("LoadingThread")
		, IOThread(nullptr)
	{
		// Start a IO thread
		IOThread = ti_new TThreadIO;
	}

	TThreadLoading::~TThreadLoading()
	{
		ti_delete IOThread;
		IOThread = nullptr;
	}

	void TThreadLoading::Start()
	{
		// Start loading thread self
		TTaskThread::Start();

		// Start IO Thread
		IOThread->Start();
	}

	void TThreadLoading::Stop()
	{
		// Stop IO Thread
		IOThread->Stop();

		// Stop loading thread self
		TTaskThread::Stop();
	}

	void TThreadLoading::AddTask(TTask* Task)
	{
		TResourceLoadingTask* LoadingTask = static_cast<TResourceLoadingTask*>(Task);
		if (LoadingTask->GetLoadingStep() == TResourceLoadingTask::STEP_IO)
		{
			// Send Task to IO Thread first
			IOThread->AddTask(Task);
		}
		else if (LoadingTask->GetLoadingStep() == TResourceLoadingTask::STEP_PARSE)
		{
			TTaskThread::AddTask(Task);
		}
		else
		{
			// Should not have other state
			TI_ASSERT(0);
		}
	}

	void TThreadLoading::OnThreadStart()
	{
		TTaskThread::OnThreadStart();

		LoadingThreadId = ThreadId;
	}
}
