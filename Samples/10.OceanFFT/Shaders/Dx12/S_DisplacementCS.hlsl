//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#define CalcDisp_RS \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=1))" 

cbuffer FInfoUniform : register(b0)
{
    float4 Info;    // x = Size
};

Texture2D<float2> IFFT_X : register(t0);

RWTexture2D<float4> DisplacementTexture : register(u0);

[RootSignature(CalcDisp_RS)]
[numthreads(32, 32, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    float Size = Info.x;

    int index = (dispatchThreadId.x + dispatchThreadId.y) % 2;
    float perm = index == 0 ? 1.f : -1.f;
    float SizeSQ_Inv = 1.f / (Size * Size);

    float z = IFFT_X.Load(int3(dispatchThreadId.xy, 0));

    DisplacementTexture[dispatchThreadId.xy] = float4(z * perm * SizeSQ_Inv, 0, 0, 0);
}
