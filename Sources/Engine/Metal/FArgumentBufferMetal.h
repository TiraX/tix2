/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_METAL

namespace tix
{
	// Metal has argument buffer
	class FArgumentBufferMetal : public FArgumentBuffer
	{
	public:
		FArgumentBufferMetal(FShaderPtr InShader);
		virtual ~FArgumentBufferMetal();
	protected:

	private:
        int32 ArgumentBindIndex;
        id<MTLBuffer> ArgumentBuffer;
        TVector<id<MTLBuffer>> Buffers;
        TVector<id<MTLTexture>> Textures;
		friend class FRHIMetal;
	};
}

#endif	// COMPILE_WITH_RHI_METAL
