/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/
#include "S_Grid2dDef.hlsli"

#define Curl_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=1))" 

Texture2D<float2> TexVelocity : register(t0);
RWTexture2D<float> TexCurl : register(u0);

[RootSignature(Curl_RootSig)]
[numthreads(16, 16, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint2 Index = dispatchThreadId.xy;

    float L = TexVelocity.Load(int3(Index.x - 1, Index.y, 0)).y;
    float R = TexVelocity.Load(int3(Index.x + 1, Index.y, 0)).y;
    float T = TexVelocity.Load(int3(Index.x, Index.y + 1, 0)).x;
    float B = TexVelocity.Load(int3(Index.x, Index.y - 1, 0)).x;
    float Vorticity = R - L - T + B;

    TexCurl[Index] = Vorticity * 0.5f;
}
