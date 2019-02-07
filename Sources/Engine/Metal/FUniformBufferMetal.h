/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_METAL

namespace tix
{
	class FUniformBufferMetal : public FUniformBuffer
	{
	public:
		FUniformBufferMetal(uint32 InStructSize);
		virtual ~FUniformBufferMetal();
	protected:

	private:
        id<MTLBuffer> ConstantBuffer;
		friend class FRHIMetal;
	};
}

#endif	// COMPILE_WITH_RHI_METAL
