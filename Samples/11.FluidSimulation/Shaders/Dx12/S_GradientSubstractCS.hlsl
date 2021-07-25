/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/
#include "S_Grid2dDef.hlsli"

#define GradientSubstract_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=2), UAV(u0, numDescriptors=1))" 

Texture2D<float> TexPressure : register(t0);
Texture2D<float2> TexVelocity : register(t1);
RWTexture2D<float2> OutTexVelocity : register(u0);

[RootSignature(GradientSubstract_RootSig)]
[numthreads(16, 16, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint2 Index = dispatchThreadId.xy;

    float L = TexPressure.Load(int3(Index.x - 1, Index.y, 0)).x;
    float R = TexPressure.Load(int3(Index.x + 1, Index.y, 0)).x;
    float T = TexPressure.Load(int3(Index.x, Index.y + 1, 0)).x;
    float B = TexPressure.Load(int3(Index.x, Index.y - 1, 0)).x;
    float2 V = TexVelocity.Load(int3(Index, 0)).xy;
    V -= float2(R - L, T - B);
    
    OutTexVelocity[Index] = V;
}
