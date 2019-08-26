//
//  Common.h
//  VirtualTexture
//
//  Created by Tirax on 2019/8/11.
//  Copyright © 2019 zhaoshuai. All rights reserved.
//

#define VT_ENABLED 0

typedef enum TiXShaderBufferIndex
{
    VBIndex_Vertices = 0,
    VBIndex_Instances = 1,
    VBIndex_View = 2,
    VBIndex_Primitive = 3,
    
    PBIndex_MaterialInstance = 0,
    PBIndex_VirtualTexture = 1,
    PBIndex_View = 2,
    PBIndex_Primitive = 3,
} TiXShaderBufferIndex;
