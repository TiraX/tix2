/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/
#include "S_Grid2dDef.hlsli"

#define AdvectionVelocity_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=1))," \
    "StaticSampler(s0, addressU = TEXTURE_ADDRESS_CLAMP, " \
                      "addressV = TEXTURE_ADDRESS_CLAMP, " \
                      "addressW = TEXTURE_ADDRESS_CLAMP, " \
                        "filter = FILTER_MIN_MAG_MIP_LINEAR)"

Texture2D<float2> TexVelocitySrc : register(t0);
RWTexture2D<float2> TexVelocityDst : register(u0);

SamplerState LinearSampler : register(s0);

[RootSignature(AdvectionVelocity_RootSig)]
[numthreads(16, 16, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint2 Index = dispatchThreadId.xy;
    float Dt = Info0.x;
    float InvSimRes = Info1.z;
    float VelDissipation = Info1.y;

    float2 UV = (Index + float2(0.5f, 0.5f)) * InvSimRes;
    float2 Vel = TexVelocitySrc.SampleLevel(LinearSampler, UV, 0).xy;
    float2 UV1 = UV - Vel * Dt * InvSimRes;
    float2 NewVel = TexVelocitySrc.SampleLevel(LinearSampler, UV1, 0).xy;

    float Decay = 1.f + VelDissipation * Dt;
    TexVelocityDst[Index] = NewVel / Decay;
}
