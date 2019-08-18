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

vertex VSOutput S_GrassVS(VSInput vsInput [[stage_in]],
                            constant EB_View & EB_View [[ buffer(1) ]])
{
    VSOutput vsOutput;
    
    float4 InPosition = float4(vsInput.position, 1.0);
    vsOutput.position = EB_View.ViewProjection * InPosition;
    
    vsOutput.normal = half3(vsInput.normal) * 2.0h - 1.0h;
    
    return vsOutput;
}

typedef struct FragmentShaderArguments {
    device float4 * Uniform [[ id(0) ]];
    texture2d<half> TexBaseColor  [[ id(1) ]];
} FragmentShaderArguments;

fragment half4 S_GrassPS(VSOutput input [[stage_in]],
                           device FragmentShaderArguments & fragmentArgs [[ buffer(0) ]])
{
    return half4(1,1,1,1);
}
