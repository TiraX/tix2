/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/
#include "S_Grid2dDef.hlsli"

#define Splat_RootSig \
	"CBV(b0) ," \
	"CBV(b1) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=1))" 

cbuffer FSplatInfo : register(b1)
{
    float4 SplatInfo0;   // xy = mouse_pos in uv space; zw = mouse move dir
    float4 SplatInfo1;   // xyz = color; z = radius scale
};

Texture2D<float2> InTexVelocity : register(t0);
RWTexture2D<float2> OutTexVelocity : register(u0);

[RootSignature(Splat_RootSig)]
[numthreads(16, 16, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint2 Index = dispatchThreadId.xy;

    float InvSimRes = Info1.z;
    float InvDyeRes = Info1.w;
    float2 MousePos = SplatInfo0.xy;
    float2 MouseDir = SplatInfo0.zw;
    float RadiusScale = SplatInfo1.w;

    float2 UVVel = (float2(Index) + 0.5f) * InvSimRes;
    float2 PVel = UVVel - MousePos;
    float2 SplatVel = MouseDir * exp(-dot(PVel, PVel) * RadiusScale);
    float2 Vel = InTexVelocity.Load(int3(Index.xy, 0));

    OutTexVelocity[Index] = Vel + SplatVel;
}
