/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TThreadLoading.h"
#include "TThreadIO.h"

namespace tix
{
	TThreadLoading* TThreadLoading::LoadingThread = nullptr;

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
}
