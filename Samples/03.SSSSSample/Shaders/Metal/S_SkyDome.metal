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
    float3 normal [[attribute(1)]];
} VSInput;

typedef struct
{
    float4 position [[position]];
    float3 normal;
} VSOutput;

typedef struct
{
    float4x4 ViewProjection;
    float3 ViewDir;
    float3 ViewPos;
} EB_View;

vertex VSOutput S_SkyDomeVS(VSInput vsInput [[stage_in]],
                            constant EB_View & EB_View [[ buffer(1) ]])
{
    VSOutput vsOutput;
    
    float4 InPosition = float4(vsInput.position, 1.0);
    vsOutput.position = EB_View.ViewProjection * InPosition;
    
    vsOutput.normal = vsInput.normal * 2.0 - 1.0;
    
    return vsOutput;
}

typedef struct FragmentShaderArguments {
    texturecube<half> texSkyMap  [[ id(0) ]];
} FragmentShaderArguments;

constexpr sampler sampler0(mip_filter::linear,
                           mag_filter::linear,
                           min_filter::linear);

fragment half4 S_SkyDomePS(VSOutput input [[stage_in]],
                           device FragmentShaderArguments & fragmentArgs [[ buffer(0) ]])
{
    float3 normal = -normalize(input.normal);
    
    half4 Color;
    Color.xyz = sqrt(fragmentArgs.texSkyMap.sample(sampler0, normal).xyz) * 0.5;
    Color.a = 1.0;
    return Color;
}
