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

		static const E_PIXEL_FORMAT DefaultBackBufferFormat = EPF_BGRA8;
		static const E_PIXEL_FORMAT DefaultDepthBufferFormat = EPF_DEPTH24_STENCIL8;
	};
}
