/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/
#include "S_Grid2dDef.hlsli"

#define Vorticity_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=2), UAV(u0, numDescriptors=1))" 

Texture2D<float> TexCurl : register(t0);
Texture2D<float2> TexVelocity : register(t1);
RWTexture2D<float2> OutTexVelocity : register(u0);

[RootSignature(Vorticity_RootSig)]
[numthreads(16, 16, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint2 Index = dispatchThreadId.xy;
    float Dt = Info0.x;
    float Curl = Info0.y;

    float L = TexCurl.Load(int3(Index.x - 1, Index.y, 0)).x;
    float R = TexCurl.Load(int3(Index.x + 1, Index.y, 0)).x;
    float T = TexCurl.Load(int3(Index.x, Index.y + 1, 0)).x;
    float B = TexCurl.Load(int3(Index.x, Index.y - 1, 0)).x;
    float C = TexCurl.Load(int3(Index.xy, 0)).x;

    float2 Force = float2(abs(T) - abs(B), abs(R) - abs(L)) * 0.5f;
    Force /= length(Force) + 0.0001f;
    Force *= Curl * C;
    Force.y *= -1.f;

    float2 Velocity = TexVelocity.Load(int3(Index.xy, 0)).xy;
    Velocity += Force * Dt;
    Velocity = clamp(Velocity, -1000.f, 1000.f);
    
    OutTexVelocity[Index] = Velocity;
}
