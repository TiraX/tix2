/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_UNIFORMBUFFER_SECTION
	{
		UB_SECTION_NORMAL,
		UB_SECTION_LIGHTS,

		UB_SECTION_COUNT,
	};
	class FRHIConfig
	{
	public: 
		static const int32 FrameBufferNum = 3;	// Use triple buffers

		static const E_PIXEL_FORMAT DefaultBackBufferFormat = EPF_BGRA8;
		static const E_PIXEL_FORMAT DefaultDepthBufferFormat = EPF_DEPTH24_STENCIL8;

		static const int32 StaticSamplerNum = 1;
	};
}
