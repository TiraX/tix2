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

vertex PixelShaderInput S_CombineVS(VertexShaderInput in [[stage_in]])
{
    PixelShaderInput out;
    
    out.position = float4(in.position, 1.0);
    out.uv = in.uv;
    
    return out;
}

typedef struct FragmentShaderArguments {
    float4 BloomParam [[ id(0) ]];    // x = exposure, y = bloom intensity,
    texture2d<half> TexSource  [[ id(1) ]];
    texture2d<half> TexBloom0  [[ id(2) ]];
    texture2d<half> TexBloom1  [[ id(3) ]];
} FragmentShaderArguments;

constexpr sampler sampler0(mip_filter::linear,
                           mag_filter::linear,
                           min_filter::linear);

half3 FilmicTonemap(half3 x) {
    half A = 0.15;
    half B = 0.50;
    half C = 0.10;
    half D = 0.20;
    half E = 0.02;
    half F = 0.30;
    //half W = 11.2;
    return ((x*(A*x + C * B) + D * E) / (x*(A*x + B) + D * F)) - E / F;
}

half3 DoToneMap(half3 color, half exposure)
{
    color = half(2.0) * FilmicTonemap(exposure * color);
    half3 whiteScale = half(1.0) / FilmicTonemap(half(11.2));
    color *= whiteScale;
    return color;
}

fragment half4 S_CombinePS(PixelShaderInput input [[stage_in]],
                           device FragmentShaderArguments & fragmentArgs [[ buffer(0) ]])
{
    const half w[] = { 2.0 / 3.0, 1.0 / 3.0 };
    half exposure = half(fragmentArgs.BloomParam.x);
    half bloomIntensity = half(fragmentArgs.BloomParam.y);
    
    //float4 color = PyramidFilter(finalTex, texcoord, pixelSize * defocus);
    half4 color = fragmentArgs.TexSource.sample(sampler0, input.uv);
    
    half4 b0 = fragmentArgs.TexBloom0.sample(sampler0, input.uv);
    half4 b1 = fragmentArgs.TexBloom1.sample(sampler0, input.uv);
    
    color.xyz += bloomIntensity * w[0] * b0.xyz;
    color.xyz += bloomIntensity * w[1] * b1.xyz;
    
    color.rgb = sqrt(DoToneMap(color.rgb, exposure));
    return color;
}
