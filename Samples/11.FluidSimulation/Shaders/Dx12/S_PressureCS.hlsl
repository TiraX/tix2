/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/
#include "S_Grid2dDef.hlsli"

#define Pressure_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=2), UAV(u0, numDescriptors=1))" 

Texture2D<float> TexDivergence : register(t0);
Texture2D<float> TexPressure : register(t1);
RWTexture2D<float> OutTexPressure : register(u0);

[RootSignature(Pressure_RootSig)]
[numthreads(16, 16, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    int2 Index = (int2)(dispatchThreadId.xy);

    float L = TexPressure.Load(int3(Index.x - 1, Index.y, 0)).x;
    float R = TexPressure.Load(int3(Index.x + 1, Index.y, 0)).x;
    float T = TexPressure.Load(int3(Index.x, Index.y + 1, 0)).x;
    float B = TexPressure.Load(int3(Index.x, Index.y - 1, 0)).x;
    float C = TexPressure.Load(int3(Index, 0)).x;

    float Div = TexDivergence.Load(int3(Index, 0)).x;
    float P = (L + R + B + T - Div) * 0.25f;

    OutTexPressure[Index] = P;
}
