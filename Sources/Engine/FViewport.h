/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include "FRHIConfig.h"

namespace tix
{
	class FViewport
	{
	public: 
		int32 Left;
		int32 Top;
		int32 Width;
		int32 Height;

		FViewport()
			: Left(0)
			, Top(0)
			, Width(0)
			, Height(0)
		{}

		FViewport(int32 X, int32 Y, int32 W, int32 H)
			: Left(X)
			, Top(Y)
			, Width(W)
			, Height(H)
		{}
	};
}
