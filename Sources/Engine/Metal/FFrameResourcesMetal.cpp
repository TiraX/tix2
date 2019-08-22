/*
 TiX Engine v2.0 Copyright (C) 2018~2019
 By ZhaoShuai tirax.cn@gmail.com
 */

#include "stdafx.h"
#include "FRHIMetal.h"
#include "FFrameResourcesMetal.h"

#if COMPILE_WITH_RHI_METAL
namespace tix
{
    FFrameResourcesMetal::FFrameResourcesMetal()
    {
    }
    
    FFrameResourcesMetal::~FFrameResourcesMetal()
    {
    }
    
    void FFrameResourcesMetal::RemoveAllReferences()
    {
        FFrameResources::RemoveAllReferences();
        
        for (auto& B : MetalBuffers)
        {
            B = nullptr;
        }
        MetalBuffers.clear();
        
        for (auto& T : MetalTextures)
        {
            T = nullptr;
        }
        MetalTextures.clear();
    }
    
    void FFrameResourcesMetal::HoldMetalBufferReference(id<MTLBuffer> InBuffer)
    {
        MetalBuffers.push_back(InBuffer);
    }
    
    void FFrameResourcesMetal::HoldMetalTextureReference(id<MTLTexture> InTexture)
    {
        MetalTextures.push_back(InTexture);
    }
}
#endif    //COMPILE_WITH_RHI_METAL
