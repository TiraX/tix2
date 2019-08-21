//
//  S_SSSBlur.metal
//  SSSSSample
//
//  Created by Tirax on 2019/1/19.
//  Copyright © 2019 zhaoshuai. All rights reserved.
//

#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

#include "VS_Instanced.h"


vertex VSOutput S_SkyVS(VertexInput vsInput [[ stage_in ]],
                        constant EB_View & EB_View [[ buffer(2) ]],
                        constant EB_Primitive & EB_Primitive [[ buffer(3) ]]
                        )
{
    VSOutput vsOutput;
    
    float3 WorldPosition = GetWorldPosition(vsInput);
    vsOutput.position = EB_View.ViewProjection * float4(WorldPosition, 1.0);
    vsOutput.texcoord0 = GetTextureCoords(vsInput);
    
    vsOutput.normal = vsInput.normal * 2.0h - 1.0h;
    vsOutput.tangent = vsInput.tangent * 2.0h - 1.0h;
    vsOutput.view = half3(EB_View.ViewPos - WorldPosition);
    
    return vsOutput;
}

typedef struct FragmentShaderArguments {
    device float4 * Uniform [[ id(0) ]];
    texture2d<half> TexBaseColor  [[ id(1) ]];
} FragmentShaderArguments;

//constexpr sampler sampler0(mip_filter::linear,
//                           mag_filter::linear,
//                           min_filter::linear);

fragment half4 S_SkyPS(VSOutput input [[stage_in]],
                           device FragmentShaderArguments & fragmentArgs [[ buffer(0) ]])
{
    constexpr sampler LinearSampler(mip_filter::linear,
                                    mag_filter::linear,
                                    min_filter::linear);
    half4 Color = fragmentArgs.TexBaseColor.sample(LinearSampler, input.texcoord0.xy);
    
    return Color;
}
