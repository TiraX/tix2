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

vertex PixelShaderInput S_AddSpecularVS(VertexShaderInput in [[stage_in]])
{
    PixelShaderInput out;
    
    out.position = float4(in.position, 1.0);
    out.uv = in.uv;
    
    return out;
}

typedef struct FragmentShaderArguments {
    texture2d<half> TexBaseColor  [[ id(0) ]];
    texture2d<half> TexSpecular  [[ id(1) ]];
} FragmentShaderArguments;

constexpr sampler sampler0(mip_filter::linear,
                           mag_filter::linear,
                           min_filter::linear);

fragment half4 S_AddSpecularPS(PixelShaderInput input [[stage_in]],
                                device FragmentShaderArguments & fragmentArgs [[ buffer(0) ]])
{
    half4 BaseColor = fragmentArgs.TexBaseColor.sample(sampler0, input.uv);
    half3 Specular = fragmentArgs.TexSpecular.sample(sampler0, input.uv).xyz;
    
    BaseColor.xyz += Specular;
    return BaseColor;
}
