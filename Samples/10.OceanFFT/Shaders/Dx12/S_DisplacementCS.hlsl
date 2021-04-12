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
    "DescriptorTable(SRV(t0, numDescriptors=6), UAV(u0, numDescriptors=2))" 

cbuffer FInfoUniform : register(b0)
{
    float4 Info;    // x = Size
};

Texture2D<float2> IFFT_X : register(t0);
Texture2D<float2> IFFT_Y : register(t1);
Texture2D<float2> IFFT_Z : register(t2);
Texture2D<float2> JacobX : register(t3);
Texture2D<float2> JacobY : register(t4);
Texture2D<float2> JacobZ : register(t5);

RWTexture2D<float4> DisplacementTexture : register(u0);

[RootSignature(CalcDisp_RS)]
[numthreads(32, 32, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    float Size = Info.x;

    int index = (dispatchThreadId.x + dispatchThreadId.y) % 2;
    float perm = index == 0 ? 1.f : -1.f;
    float SizeSQ_Inv = 1.f / (Size * Size);

    float3 D;
    D.x = IFFT_X.Load(int3(dispatchThreadId.xy, 0)).x;
    D.y = IFFT_Y.Load(int3(dispatchThreadId.xy, 0)).x;
    D.z = -IFFT_Z.Load(int3(dispatchThreadId.xy, 0)).x;

    D *= perm * SizeSQ_Inv;


    float3 J;
    J.x = JacobX.Load(int3(dispatchThreadId.xy, 0)).x;
    J.y = JacobY.Load(int3(dispatchThreadId.xy, 0)).x;
    J.z = JacobZ.Load(int3(dispatchThreadId.xy, 0)).x;

    J *= perm / Size;
    J.xy += 1.f;

    // eigenvalue & eigenvector
    float a, b;
    a = J.x + J.y;
    b = sqrt((J.x - J.y) * (J.x - J.y) + 4.f * J.z * J.z);
    float jminus, jplus;
    jminus = (a - b) * 0.5f;
    jplus = (a + b) * 0.5f;
    float qplus = (jplus - J.x) / J.z;
    float qminus = (jminus - J.x) / J.z;

    a = sqrt(1.f + qplus * qplus);
    b = sqrt(1.f + qminus * qminus);

    float3 EMinus = float3(1.f / b, 0.f, qminus / b);

    DisplacementTexture[dispatchThreadId.xy] = float4(D.xyz, jminus);
}
