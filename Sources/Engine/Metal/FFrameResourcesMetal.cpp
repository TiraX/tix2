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
    }
    
    void FFrameResourcesMetal::HoldMetalBufferReference(id<MTLBuffer> InBuffer)
    {
        MetalBuffers.push_back(InBuffer);
    }
}
#endif    //COMPILE_WITH_RHI_METAL
