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

#define HKt_RootSig \
    "RootConstants(num32BitConstants=4, b0), " \
	"CBV(b1) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=6))" 

static const float PI = 3.14159f;
static const float GRAVITY = 9.8f;

float4 GameTime : register(b0);

cbuffer FInfoUniform : register(b1)
{
    float4 Info;    // x = Size; y = L; z = Depth; w = ChopScale;
};

Texture2D<float4> H0Texture : register(t0);
RWTexture2D<float2> HKtX : register(u0);
RWTexture2D<float2> HKtY : register(u1);
RWTexture2D<float2> HKtZ : register(u2);
RWTexture2D<float2> JxxResult : register(u3);
RWTexture2D<float2> JyyResult : register(u4);
RWTexture2D<float2> JxyResult : register(u5);

float2 complex_mul(in float2 c0, in float2 c1)
{
    float2 c;
    c.x = c0.x * c1.x - c0.y * c1.y;
    c.y = c0.x * c1.y + c0.y * c1.x;
    return c;
}

[RootSignature(HKt_RootSig)]
[numthreads(32, 32, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const float GridSize = Info.x;
    const float L = Info.y;
    const float Depth = Info.z;
    const float ChopScale = Info.w;
    const float Time = GameTime.x;

    float2 k = (float2(dispatchThreadId.xy) - GridSize * 0.5f) * PI * 2.f / L;

    // Load h0 and h0m
    float4 H0 = H0Texture.Load(int3(dispatchThreadId.xy, 0));
    float2 h0k = H0.xy;
    float2 h0mk = float2(H0.z, -H0.w);  // conj

    // omega
    float kl = length(k);
    //float omega = sqrt(GRAVITY * kl * tanh(kl * Depth));
    float omega = sqrt(GRAVITY * kl);

    float OT = omega * Time;
    float sinOT, cosOT;
    sincos(OT, sinOT, cosOT);

    // Height
    float2 HKt_Z = complex_mul(h0k, float2(cosOT, sinOT)) +
                    complex_mul(h0mk, float2(cosOT, -sinOT));
    // X and Y choppy
    float2 HKt_X, HKt_Y, Jxx, Jyy, Jxy;
    if (kl == 0.f)
    {
        HKt_X = float2(0.f, 0.f);
        HKt_Y = float2(0.f, 0.f);
        Jxx = float2(0.f, 0.f);
        Jyy = float2(0.f, 0.f);
        Jxy = float2(0.f, 0.f);
    }
    else
    {
        HKt_X = complex_mul(HKt_Z * ChopScale * k.x / kl, float2(0, -1));
        HKt_Y = complex_mul(HKt_Z * ChopScale * k.y / kl, float2(0, -1));
        Jxx = HKt_Z * ChopScale * k.x * k.x / kl;
        Jyy = HKt_Z * ChopScale * k.y * k.y / kl;
        Jxy = HKt_Z * ChopScale * k.x * k.y / kl;
    }

    // Output
    HKtX[dispatchThreadId.xy] = HKt_X;
    HKtY[dispatchThreadId.xy] = HKt_Y;
    HKtZ[dispatchThreadId.xy] = HKt_Z;
    JxxResult[dispatchThreadId.xy] = Jxx;
    JyyResult[dispatchThreadId.xy] = Jyy;
    JxyResult[dispatchThreadId.xy] = Jxy;
}
