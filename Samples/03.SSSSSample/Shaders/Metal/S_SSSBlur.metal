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

vertex PixelShaderInput S_SSSBlurVS(VertexShaderInput vsInput [[stage_in]])
{
    PixelShaderInput output;
    float4 pos = float4(vsInput.position, 1.0f);
    
    // vertex input position already in projected space.
    output.position = pos;
    
    output.uv = vsInput.uv;
    
    return output;
}

#define SSSS_N_SAMPLES (17)

typedef struct FragmentShaderArguments {
    float4 BlurDir [[ id(0) ]];
    float4 BlurParam [[ id(1) ]];   // x = sssWidth; y = sssFov; z = maxOffsetMm
    device float4 *Kernel [[ id(2) ]];
    texture2d<half> TexColor  [[ id(3) ]];
    texture2d<half> TexStrength  [[ id(4) ]];
} FragmentShaderArguments;

constexpr sampler sampler0(mip_filter::linear,
                           mag_filter::linear,
                           min_filter::linear);

fragment half4 S_SSSBlurPS(PixelShaderInput input [[stage_in]],
                           device FragmentShaderArguments & fragmentArgs [[ buffer(0) ]])
{
    // Fetch color of current pixel:
    half4 colorM = fragmentArgs.TexColor.sample(sampler0, input.uv);
    
    // Fetch linear depth of current pixel:
    half depthM = colorM.a;// TexDepth.Sample(sampler0, input.uv).r;
    
    half sssWidth = half(fragmentArgs.BlurParam.x);
    half sssFov = half(fragmentArgs.BlurParam.y);
    half maxOffsetMm = half(fragmentArgs.BlurParam.z);
    
    // Calculate the sssWidth scale (1.0 for a unit plane sitting on the
    // projection window):
    half distanceToProjectionWindow = half(1.0) / tan(half(0.5) * sssFov);
    half scale = distanceToProjectionWindow / depthM;
    
    half2 dir = half2(fragmentArgs.BlurDir.xy);
    
    // Calculate the final step to fetch the surrounding pixels:
    half2 finalStep = scale * dir;
    finalStep *= fragmentArgs.TexStrength.sample(sampler0, input.uv).a; // Modulate it using the alpha channel.
    finalStep *= half(1.0) / (half(2.0) * sssWidth); // sssWidth in mm / world space unit, divided by 2 as uv coords are from [0 1]
    
    // Accumulate the center sample:
    half4 colorBlurred = colorM;
    colorBlurred.rgb *= half3(fragmentArgs.Kernel[0].rgb);
    
    // Accumulate the other samples:
    for (int i = 1; i < SSSS_N_SAMPLES; i++) {
        // Fetch color and depth for current sample:
        float2 offset = input.uv + fragmentArgs.Kernel[i].a * float2(finalStep);
        half4 color = fragmentArgs.TexColor.sample(sampler0, offset);
        
        //#if SSSS_FOLLOW_SURFACE == 1
        // If the difference in depth is huge, we lerp color back to "colorM":
        half depth = color.a;// TexDepth.Sample(sampler0, offset).r;
        
        half s = saturate(abs(depthM - depth) / (distanceToProjectionWindow * (maxOffsetMm / sssWidth)));
        s = min(half(1.0), s * half(1.5)); // custom / user definable scaling
        
        color.rgb = mix(color.rgb, colorM.rgb, s);
        //#endif
        
        // Accumulate:
        colorBlurred.rgb += half3(fragmentArgs.Kernel[i].rgb) * color.rgb;
    }
    
    return colorBlurred;
}
