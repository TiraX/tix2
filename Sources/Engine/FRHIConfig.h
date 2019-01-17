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
#if COMPILE_WITH_RHI_DX12
		static const E_PIXEL_FORMAT DefaultDepthBufferFormat = EPF_DEPTH24_STENCIL8;
		static const E_PIXEL_FORMAT DefaultStencilBufferFormat = EPF_UNKNOWN;
#elif COMPILE_WITH_RHI_METAL
		static const E_PIXEL_FORMAT DefaultDepthBufferFormat = EPF_DEPTH16;
		static const E_PIXEL_FORMAT DefaultStencilBufferFormat = EPF_STENCIL8;
#else
	#error("unknown platform.")
#endif

		static const int32 StaticSamplerNum = 1;
	};
}
