/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_METAL

namespace tix
{
	class FTextureMetal : public FTexture
	{
	public:
		FTextureMetal();
		FTextureMetal(const TTextureDesc& Desc);
		virtual ~FTextureMetal();

        id<MTLTexture> GetMetalTexture()
        {
            return Texture;
        }
        
	protected:

    private:
        id <MTLTexture> Texture;
		friend class FRHIMetal;
	};
}

#endif	// COMPILE_WITH_RHI_METAL
