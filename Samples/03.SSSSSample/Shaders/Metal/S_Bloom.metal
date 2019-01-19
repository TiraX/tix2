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

vertex ColorInOut S_BloomVS(Vertex in [[stage_in]])
{
    ColorInOut out;
    
    out.position = float4(in.position, 1.0);
    out.texCoord = in.texCoord;
    
    return out;
}

fragment float4 S_BloomPS(ColorInOut in [[stage_in]],
                             texture2d<half> texture [[ texture(0) ]])
{
    constexpr sampler colorSampler(mip_filter::linear,
                                   mag_filter::linear,
                                   min_filter::linear);
    
    half4 colorSample = texture.sample(colorSampler, in.texCoord.xy);
    
    return float4(colorSample);
}
