/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/
#include "S_Grid2dDef.hlsli"

#define ClearPressure_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=1))" 

Texture2D<float> TexPressure : register(t0);
RWTexture2D<float> OutTexPressure : register(u0);

[RootSignature(ClearPressure_RootSig)]
[numthreads(16, 16, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    int2 Index = int2(dispatchThreadId.xy);
    float PressureFade = Info1.x;

    float Pressure = TexPressure.Load(int3(Index.xy, 0)).x;
    OutTexPressure[Index] = Pressure * PressureFade;
}
