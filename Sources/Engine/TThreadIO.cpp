/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TThreadIO.h"

namespace tix
{
	TThreadIO::TThreadIO()
		: TTaskThread("IOThread")
	{
	}

	TThreadIO::~TThreadIO()
	{
	}
}
