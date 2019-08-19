//
//  M_FullScreen.metal
//  TiX2
//
//  Created by Tirax on 2019/1/17.
//  Copyright © 2019 zhaoshuai. All rights reserved.
//

#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

typedef struct
{
    float3 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
} Vertex;

typedef struct
{
    float4 position [[position]];
    float2 texCoord;
} ColorInOut;

vertex ColorInOut FullScreenVS(Vertex in [[stage_in]])
{
    ColorInOut out;
    
    out.position = float4(in.position, 1.0);
    out.texCoord = in.texCoord;
    
    return out;
}

typedef struct FragmentShaderArguments {
    texture2d<half> TexSource  [[ id(0) ]];
} FragmentShaderArguments;

fragment half4 FullScreenPS(ColorInOut in [[stage_in]],
                            constant FragmentShaderArguments & fragmentArgs [[ buffer(0) ]])
{
    constexpr sampler colorSampler(mip_filter::nearest,
                                   mag_filter::nearest,
                                   min_filter::nearest);
    
    half4 colorSample = fragmentArgs.TexSource.sample(colorSampler, in.texCoord.xy);
    
    return colorSample;
}


