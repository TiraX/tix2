/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_METAL

namespace tix
{
    class FRenderTargetMetal : public FRenderTarget
    {
    public:
        FRenderTargetMetal(int32 W, int32 H);
        virtual ~FRenderTargetMetal();
        
    protected:

    private:
        MTLRenderPassDescriptor * RenderPassDesc;
        friend class FRHIMetal;
    };
}

#endif	// COMPILE_WITH_RHI_METAL
