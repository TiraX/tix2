/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/
#include "S_Grid2dDef.hlsli"

#define AdvectionDye_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=2), UAV(u0, numDescriptors=1)), " \
    "StaticSampler(s0, addressU = TEXTURE_ADDRESS_CLAMP, " \
                      "addressV = TEXTURE_ADDRESS_CLAMP, " \
                      "addressW = TEXTURE_ADDRESS_CLAMP, " \
                        "filter = FILTER_MIN_MAG_MIP_LINEAR)"

Texture2D<float2> TexVelocity : register(s0);
Texture2D<float4> TexDyeSrc : register(s1);
RWTexture2D<float4> TexDyeDst : register(u0);

SamplerState LinearSampler : register(s0);

// float2 BilinearSample(in Texture2D<float2> InTex, in float2 UV, in float TexelSize)
// {
//     float2 st = UV / TexelSize;

//     float2 UV0 = floor(st);
//     int4 iUV = int4(int2(UV0), int2( UV0 + float2(1,1) ));
//     float2 fuv = fract(st);

//     float2 a = InTex.Load(iUV.xy);
//     float2 b = InTex.Load(iUV.zy);
//     float2 c = InTex.Load(iUV.xw);
//     float2 d = InTex.Load(iUV.zw);

//     return lerp(lerp(a, b, fuv.x), lerp(c, d, fuv.x), fuv.y);
// }

[RootSignature(AdvectionDye_RootSig)]
[numthreads(16, 16, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint2 Index = dispatchThreadId.xy;
    float Dt = Info0.x;
    float InvSimRes = Info1.z;
    float InvDyeRes = Info1.w;
    float DyeDissipation = Info1.w;

    float2 UV = (Index + float2(0.5f, 0.5f)) * InvDyeRes;
    float2 Vel = TexVelocity.SampleLevel(LinearSampler, UV, 0).xy;
    float2 UV1 = UV - Vel * Dt * InvSimRes;
    float4 NewDye = TexDyeSrc.SampleLevel(LinearSampler, UV1, 0);

    float Decay = 1.f + DyeDissipation * Dt;
    TexDyeDst[Index] = NewDye / Decay;
}
