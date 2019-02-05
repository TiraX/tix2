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

vertex PixelShaderInput S_BloomVS(VertexShaderInput in [[stage_in]])
{
    PixelShaderInput out;
    
    out.position = float4(in.position, 1.0);
    out.uv = in.uv;
    
    return out;
}

typedef struct FragmentShaderArguments {
    float4 BloomParam [[ id(0) ]];    // xy = bloom step
    texture2d<half> TexSource  [[ id(1) ]];
} FragmentShaderArguments;

constexpr sampler sampler0(mip_filter::linear,
                           mag_filter::linear,
                           min_filter::linear);

fragment half4 S_BloomPS(PixelShaderInput input [[stage_in]],
                          device FragmentShaderArguments & fragmentArgs [[ buffer(0) ]])
{
    //#if N_SAMPLES == 13
    //    float offsets[] = { -1.7688, -1.1984, -0.8694, -0.6151, -0.3957, -0.1940, 0, 0.1940, 0.3957, 0.6151, 0.8694, 1.1984, 1.7688 };
    //    const float n = 13.0;
    //#elif N_SAMPLES == 11
    //    float offsets[] = { -1.6906, -1.0968, -0.7479, -0.4728, -0.2299, 0, 0.2299, 0.4728, 0.7479, 1.0968, 1.6906 };
    //    const float n = 11.0;
    //#elif N_SAMPLES == 9
    //    float offsets[] = { -1.5932, -0.9674, -0.5895, -0.2822, 0, 0.2822, 0.5895, 0.9674, 1.5932 };
    //    const float n = 9.0;
    //#elif N_SAMPLES == 7
        const half offsets[] = { -1.4652, -0.7916, -0.3661, 0, 0.3661, 0.7916, 1.4652 };
        #define    N_SAMPLE 7
    const half avg_inv = half(1.0) / half(N_SAMPLE);
    //#else
    //    float offsets[] = { -1.282, -0.524, 0.0, 0.524, 1.282 };
    //    const float n = 5.0;
    //#endif
    
    half2 step = half2(fragmentArgs.BloomParam.xy);
    
    half4 color = half4(0.0, 0.0, 0.0, 0.0);
    //[unroll]
    for (int i = 0; i < N_SAMPLE; i++)
        color += fragmentArgs.TexSource.sample(sampler0, float2(half2(input.uv) + step * offsets[i]));
    return color * avg_inv;
}
