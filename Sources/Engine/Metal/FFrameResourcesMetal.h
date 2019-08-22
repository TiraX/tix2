/*
 TiX Engine v2.0 Copyright (C) 2018~2019
 By ZhaoShuai tirax.cn@gmail.com
 */

#pragma once

#if COMPILE_WITH_RHI_METAL

namespace tix
{
    // Frame resources hold all the resources(textures, buffers, shaders etc), used in this frame, make sure they are not released until GPU done with them
    class FFrameResourcesMetal : public FFrameResources
    {
    public:
        FFrameResourcesMetal();
        virtual ~FFrameResourcesMetal();
        
        virtual void RemoveAllReferences();
        
        void HoldMetalBufferReference(id<MTLBuffer> InBuffer);
        void HoldMetalTextureReference(id<MTLTexture> InTexture);
        
    private:
        // Hold some temp resources used in this frame
        TList< id<MTLBuffer> > MetalBuffers;
        TList< id<MTLTexture> > MetalTextures;
    };
}

#endif //COMPILE_WITH_RHI_METAL
