//
//  S_Grass.metal
//  VirtualTexture
//
//  Created by Tirax on 2019/8/11.
//  Copyright © 2019 zhaoshuai. All rights reserved.
//

#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

#include "VS_Instanced.h"

vertex VSOutput S_EmptyVTVS(VSInput vsInput [[stage_in]],
                            constant EB_View & EB_View [[ buffer(1) ]])
{
    VSOutput vsOutput;
    
    float4 InPosition = float4(vsInput.position, 1.0);
    vsOutput.position = EB_View.ViewProjection * InPosition;
    
    vsOutput.normal = half3(vsInput.normal) * 2.0h - 1.0h;
    
    return vsOutput;
}

typedef struct FragmentShaderArguments {
    texture2d<half> IndirectTexture  [[ id(0) ]];
    texture2d<half> PhysicPageTexture  [[ id(1) ]];
} VTArguments;

fragment half4 S_EmptyVTPS(VSOutput input [[stage_in]],
                           device VTArguments & fragmentArgs [[ buffer(0) ]])
{
    return half4(1,1,1,1);
}
