/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FStats
	{
	public:
		static FStats Stats;

		FStats();
		~FStats();

		static void ResetPerFrame();

		uint32 VertexDataInBytes;
		uint32 IndexDataInBytes;
		uint32 TrianglesRendered;
	};
} // end namespace tix
