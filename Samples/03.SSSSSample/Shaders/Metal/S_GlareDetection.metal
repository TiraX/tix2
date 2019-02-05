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
    float2 uv [[attribute(1)]];
} VertexShaderInput;

typedef struct
{
    float4 position [[position]];
    float2 uv;
} PixelShaderInput;

vertex PixelShaderInput S_GlareDetectionVS(VertexShaderInput in [[stage_in]])
{
    PixelShaderInput out;
    
    out.position = float4(in.position, 1.0);
    out.uv = in.uv;
    
    return out;
}

typedef struct FragmentShaderArguments {
    float4 GlareDetectionParam [[ id(0) ]];    // x=exposure, y=threshold,
    texture2d<half> TexBaseColor  [[ id(1) ]];
} FragmentShaderArguments;

constexpr sampler sampler0(mip_filter::linear,
                           mag_filter::linear,
                           min_filter::linear);

fragment half4 S_GlareDetectionPS(PixelShaderInput input [[stage_in]],
                                  device FragmentShaderArguments & fragmentArgs [[ buffer(0) ]])
{
    
    half exposure = half(fragmentArgs.GlareDetectionParam.x);
    half bloomThreshold = half(fragmentArgs.GlareDetectionParam.y);
    
    half4 color = fragmentArgs.TexBaseColor.sample(sampler0, input.uv);
    color.rgb *= exposure;
    
    return half4(max(color.rgb - bloomThreshold / (half(1.0) - bloomThreshold), half(0.0)), color.a);
}
