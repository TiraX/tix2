/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/
#include "S_Grid2dDef.hlsli"

#define Divergence_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=1))" 

Texture2D<float2> TexVelocity : register(t0);
RWTexture2D<float> TexDivergence : register(u0);

[RootSignature(Divergence_RootSig)]
[numthreads(16, 16, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint2 Index = dispatchThreadId.xy;
    uint SimRes = (uint)(Info0.z);

    float L = TexVelocity.Load(int3(Index.x - 1, Index.y, 0)).x;
    float R = TexVelocity.Load(int3(Index.x + 1, Index.y, 0)).x;
    float T = TexVelocity.Load(int3(Index.x, Index.y + 1, 0)).y;
    float B = TexVelocity.Load(int3(Index.x, Index.y - 1, 0)).y;
    float2 C = TexVelocity.Load(int3(Index.xy, 0)).xy;

    L = (Index.x == 0) ? -C.x : L;
    R = (Index.x == (SimRes - 1)) ? -C.x : R;
    T = (Index.y == (SimRes - 1)) ? -C.y : T;
    B = (Index.y == 0) ? -C.y : B;

    float Div = (R - L + T - B) * 0.5f;
    TexDivergence[Index] = Div;
}
