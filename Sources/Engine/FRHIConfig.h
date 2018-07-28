/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRHIConfig
	{
	public: 
		static const int32 FrameBufferNum = 3;	// Use triple buffers
		static const int32 MultiRTMax = 4;	// max multi-render targets count
	};
}
