/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/
#include "S_Grid2dDef.hlsli"

#define Splat_RootSig \
	"CBV(b0) ," \
	"CBV(b1) ," \
    "DescriptorTable(UAV(u0, numDescriptors=1))" 

cbuffer FSplatInfo : register(b1)
{
    float4 SplatInfo0;   // xy = mouse_pos in uv space; zw = mouse move dir
    float4 SplatInfo1;   // xyz = color; z = radius scale
};

RWTexture2D<float4> TexDye : register(u0);

[RootSignature(Splat_RootSig)]
[numthreads(16, 16, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint2 Index = dispatchThreadId.xy;

    float InvSimRes = Info1.z;
    float InvDyeRes = Info1.w;
    float2 MousePos = SplatInfo0.xy;
    float3 Color = SplatInfo1.xyz;
    float RadiusScale = SplatInfo1.w;

    float2 UVDye = (float2(Index) + 0.5f) * InvDyeRes;
    float2 PDye = UVDye - MousePos;
    float3 SplatDye = Color * exp(-dot(PDye, PDye) * RadiusScale);

    TexDye[Index] += float4(SplatDye.xyz, 0.f);
}
