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

#define HZero_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=1))" 

static const float PI = 3.14159f;

cbuffer FInfoUniform : register(b0)
{
    float4 Info;    // x = Size; y = L; z = windSpeed ^ 2 / g; w = A;
    float4 Info2;   // xy = normalized_wind_direction; z = l(spress value); w = damp;
};

Texture2D<float4> GaussRandTexture : register(t0);
RWTexture2D<float4> H0Result : register(u0);

float PhillipsSpectrum(in float2 k)
{
    float _L = Info.z;  // WindSpeed * WindSpeed / g
    float A = Info.w;
    float2 WindDir = Info2.xy;
    float l = Info2.z;
    float damp = Info2.w;
    
    float result = 0.0;
    float k2 = dot(k, k);
    if (k2 > 0.0)
    {
        float k4_inv = 1.f / (k2 * k2);
        float2 kn = normalize(k);
        float KoW = dot(kn, WindDir);

        if (KoW < 0.0)
            KoW *= damp;

        result = A
             * exp(-1.f / (k2 * _L * _L)) 
             * exp(-k2 * l * l)
             * (KoW * KoW)
             * k4_inv;
    }
    return result;
}

static const float INV_SQRT_2 = 1.f / sqrt(2.f);

[RootSignature(HZero_RootSig)]
[numthreads(32, 32, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    float GridSize = Info.x;
    float L = Info.y;

    float2 x = float2(dispatchThreadId.xy) - GridSize * 0.5;
    float2 k = x * 2.f * PI / L;

    float4 guassRand = GaussRandTexture.Load(int3(dispatchThreadId.xy, 0));

    float2 h0 = guassRand.xy * sqrt(PhillipsSpectrum(k))  * INV_SQRT_2;
    float2 h0m = guassRand.zw * sqrt(PhillipsSpectrum(-k)) * INV_SQRT_2;

    H0Result[dispatchThreadId.xy] = float4(h0, h0m);
}
