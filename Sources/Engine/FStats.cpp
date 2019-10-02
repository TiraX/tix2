/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FStats.h"

namespace tix
{
	FStats FStats::Stats;

	FStats::FStats()
		: VertexDataInBytes(0)
		, IndexDataInBytes(0)
		, TrianglesRendered(0)
	{
	}

	FStats::~FStats()
	{
	}

	void FStats::ResetPerFrame()
	{
		Stats.TrianglesRendered = 0;
	}
}
