/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TThreadIO.h"

namespace tix
{
	TThreadId TThreadIO::IOThreadId;

	TThreadIO::TThreadIO()
		: TTaskThread("IOThread")
	{
	}

	TThreadIO::~TThreadIO()
	{
	}

	void TThreadIO::OnThreadStart()
	{
		TTaskThread::OnThreadStart();

		IOThreadId = ThreadId;
	}
}
