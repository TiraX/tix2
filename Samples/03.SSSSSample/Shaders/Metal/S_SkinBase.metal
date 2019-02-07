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
    float2 texcoord0 [[attribute(2)]];
    float3 tangent [[attribute(3)]];
} VSInput;

typedef struct
{
    float4 position [[position]];
    float2 texCoord;
    half3 normal;
    half3 tangent;
    half3 view;
    float4 worldPosition;
} VSOutput;

typedef struct
{
    float4x4 ViewProjection;
    float3 ViewDir;
    float3 ViewPos;
} EB_View;

typedef struct
{
    int4 LightCount;
    int4 LightIndex;
} EB_Primitive;

#define MAX_LIGHTS (32)

typedef struct
{
    float4 LightPosition[MAX_LIGHTS];
    float4 LightColor[MAX_LIGHTS];
} EB_Lights;


vertex VSOutput S_SkinBaseVS(VSInput vsInput [[stage_in]],
                             constant EB_View & EB_View [[ buffer(0) ]])
{
    VSOutput vsOutput;
    
    vsOutput.position = float4(vsInput.position, 1.0);
    vsOutput.position = EB_View.ViewProjection * vsOutput.position;
    vsOutput.texCoord = vsInput.texcoord0;
    vsOutput.texCoord.y = 1.0 - vsOutput.texCoord.y;
    
    vsOutput.normal = half3(vsInput.normal * 2.0 - 1.0);
    vsOutput.tangent = half3(vsInput.tangent * 2.0 - 1.0);
    vsOutput.view = half3(normalize(EB_View.ViewPos - vsInput.position));
    vsOutput.worldPosition.xyz = vsInput.position;
    vsOutput.worldPosition.w = vsOutput.position.z / vsOutput.position.w;
    
    return vsOutput;
}

typedef struct FragmentShaderArguments {
    texture2d<half> texDiffuse  [[ id(0)  ]];
    texture2d<half> texNormal  [[ id(1)  ]];
    texture2d<half> texSpecular  [[ id(2)  ]];
    texture2d<half> texBeckmann  [[ id(3)  ]];
    texturecube<half> texIrrMap  [[ id(4)  ]];
} FragmentShaderArguments;

constexpr sampler sampler0(mip_filter::linear,
                           mag_filter::linear,
                           min_filter::linear);

static constant half specularIntensity = 1.84;
//static constant half specularRoughness = 0.3;
static constant half specularFresnel = 0.81;

//half3 BumpMap(texture2d<half> normalTex, half2 texcoord) {
//    half3 bump;
//    bump.xy = -1.0 + 2.0 * normalTex.sample(sampler0, texcoord).gr;
//    bump.z = sqrt(1.0 - bump.x * bump.x - bump.y * bump.y);
//    return normalize(bump);
//}

half Fresnel(half3 h, half3 view, half f0) {
    half base = 1.0 - dot(view, h);
    half exponential = pow(base, half(5.0));
    return exponential + f0 * (1.0 - exponential);
}

half SpecularKSK(texture2d<half> beckmannTex, half3 normal, half3 light, half3 view, half roughness) {
    half3 h = view + light;
    half3 halfn = normalize(h);
    
    half ndotl = max(dot(normal, light), half(0.0));
    half ndoth = max(dot(normal, halfn), half(0.0));
    
    half ph = pow(half(2.0) * beckmannTex.sample(sampler0, float2(ndoth, roughness), 0).r, half(10.0));
    half f = mix(half(0.25), Fresnel(halfn, view, 0.028), specularFresnel);
    half ksk = max(ph * f / dot(h, h), half(0.0));
    
    return ndotl * ksk;
}

half3 GetWorldSpaceNormal(half3 objSpaceNormal, VSOutput input)
{
    half3 iNormal = normalize(input.normal);
    half3 iTangent = normalize(input.tangent);
    half3 iBitangent = cross(iNormal, iTangent);
    
    matrix_half3x3 tbn = matrix_half3x3(iTangent, iBitangent, iNormal);
    return tbn * objSpaceNormal;
    //float3x3 tbn = transpose(float3x3(iTangent, iBitangent, iNormal));
    //return mul(tbn, objSpaceNormal);
}

struct SSSSFragmentOutput {
    // color attachment 0
    half4 BaseColor [[ color(0) ]];
    
    // color attachment 1
    half4 Specular [[ color(1) ]];
};

fragment SSSSFragmentOutput S_SkinBasePS(VSOutput input [[stage_in]],
                             constant EB_Primitive & EB_Primitive [[ buffer(0) ]],
                             constant EB_Lights & EB_Lights [[ buffer(1) ]],
                             device FragmentShaderArguments & fragmentArgs [[ buffer(2) ]])
{
    half3 objSpaceNormal = normalize(fragmentArgs.texNormal.sample(sampler0, float2(input.texCoord)).xyz * half(2.0) - half(1.0));
    
    half3 worldSpaceNormal = GetWorldSpaceNormal(objSpaceNormal, input);
    half3 normal = worldSpaceNormal;
    
    half3 view = normalize(input.view);
    
    // Fetch albedo, specular parameters and static ambient occlusion:
    half4 albedo = fragmentArgs.texDiffuse.sample(sampler0, input.texCoord);
    half3 specularAO = fragmentArgs.texSpecular.sample(sampler0, input.texCoord).rgb;
    
    half occlusion = specularAO.r;
    half intensity = specularAO.b * specularIntensity;
    half roughness = specularAO.g;//(specularAO.g / 0.3) * 0.3;// specularRoughness;
    
    // Initialize the output:
    half4 color = half4(0.0, 0.0, 0.0, 0.0);
    half4 specularColor = half4(0.0, 0.0, 0.0, 0.0);
    
    //*
    //[unroll]
    for (int i = 0; i < EB_Primitive.LightCount.x; i++)
    {
        int Index = EB_Primitive.LightIndex[i];
        half3 LColor = half3(EB_Lights.LightColor[Index].xyz);
        float3 light = EB_Lights.LightPosition[Index].xyz - input.worldPosition.xyz;
        half distanceSqr = dot(light, light);
        half3 L = half3(light * rsqrt(distanceSqr));
        
        //        float spot = dot(lights[i].direction, -light);
        //        [flatten]
        //        if (spot > lights[i].falloffStart) {
        //            // Calculate attenuation:
        //            float curve = min(pow(dist / lights[i].farPlane, 6.0), 1.0);
        //            float attenuation = lerp(1.0 / (1.0 + lights[i].attenuation * dist * dist), 0.0, curve);
        half attenuation = half(1.0) / (distanceSqr + half(1.0));
        //
        //            // And the spot light falloff:
        //            spot = saturate((spot - lights[i].falloffStart) / lights[i].falloffWidth);
        //
        //            // Calculate some terms we will use later on:
        half3 f1 = LColor * attenuation;
        half3 f2 = albedo.rgb * f1;
        
        // Calculate the diffuse and specular lighting:
        half3 diffuse = saturate(dot(L, normal));
        half specular = intensity * SpecularKSK(fragmentArgs.texBeckmann, normal, L, view, roughness);
        
        // And also the shadowing:
        half shadow = 1.0;// ShadowPCF(input.worldPosition, i, 3, 1.0);
        
        // Add the diffuse and specular components:
        //#ifdef SEPARATE_SPECULARS
        color.rgb += shadow * f2 * diffuse;
        specularColor.rgb += shadow * f1 * specular;
        //#else
        //        color.rgb += shadow * (f2 * diffuse + f1 * specular);
        //#endif
        
        // Add the transmittance component:
        //if (sssEnabled && translucencyEnabled)
        //    color.rgb += f2 * albedo.a * SSSSTransmittance(translucency, sssWidth, input.worldPosition, input.normal, light, shadowMaps[i], lights[i].viewProjection, lights[i].farPlane);
        //        }
    }
    //*/
    
    // Add the ambient component:
    //color.rgb += occlusion * ambient * albedo.rgb * irradianceTex.Sample(LinearSampler, normal).rgb;
    //float3 dir = float3(-normal.x, -normal.y, -normal.z);
    float3 dir = float3(normal.xzy);
    color.rgb += occlusion * albedo.rgb * half(0.61) * sqrt(fragmentArgs.texIrrMap.sample(sampler0, dir).rgb);
    
    // Store the SSS strength:
    specularColor.a = albedo.a;
    
    // Store the depth value:
    //depth = input.svPosition.w;
    
    // Convert to non-homogeneous points by dividing by w:
    //input.currPosition.xy /= input.currPosition.z; // w is stored in z
    //input.prevPosition.xy /= input.prevPosition.z;
    
    // Calculate velocity in non-homogeneous projection space:
    //velocity = input.currPosition.xy - input.prevPosition.xy;
    
    // Compress the velocity for storing it in a 8-bit render target:
    //color.a = sqrt(5.0 * length(velocity));
    color.a = input.worldPosition.w;
    
    SSSSFragmentOutput Output;
    Output.BaseColor = color;
    Output.Specular = specularColor;
    
    return Output;
}
