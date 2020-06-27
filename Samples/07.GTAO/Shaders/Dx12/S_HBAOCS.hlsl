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

#define HBAO_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=2), UAV(u0, numDescriptors=1))," \
    "StaticSampler(s0, addressU = TEXTURE_ADDRESS_CLAMP, " \
                      "addressV = TEXTURE_ADDRESS_CLAMP, " \
                      "addressW = TEXTURE_ADDRESS_CLAMP, " \
                        "filter = FILTER_MIN_MAG_MIP_POINT)"

static const float PI = 3.14159f;
static const int MAX_DIR = 16;
static const float MAX_DIR_INV = 1.0 / float(MAX_DIR);
static const int MAX_STEPS = 12;
static const float STEP_LENGTH = 2.f;
static const float ANGLE_STEP = PI * 2.f / MAX_DIR;

cbuffer FInfoUniform : register(b0)
{
    float4 ScreenSize;  // xy = Size; zw = InvSize;
    float4 FocalLen;    // xy = FocalLen; zw = InvFocalLen;
    float4 Radius;      // x = radius; y = radius^2; z = 1.0/radius
};

Texture2D<float4> SceneNormal : register(t0);
Texture2D<float> SceneDepth : register(t1);

RWTexture2D<float> AOResult : register(u0);

SamplerState PointSampler : register(s0);


float3 ScreenToViewPos(float2 UV, float Depth)
{
	UV = UV * float2(2.0, 2.0) - float2(1.0, 1.0);
	return float3(UV * FocalLen.zw * Depth, Depth);
}

float3 ScreenToViewPos(float2 UV)
{
    float Depth = SceneDepth.SampleLevel(PointSampler, UV, 0);
	return ScreenToViewPos(UV, Depth);
}

float tangent(float3 T)
{
    return -T.z / length(T.xy);
}

float tangent(float3 P, float3 S)
{
    return (P.z - S.z) / length(S.xy - P.xy);
}

float3 MinDiff(float3 P, float3 R, float3 L)
{
    float3 V1 = R - P;
    float3 V2 = P - L;
    return (dot(V1, V1) < dot(V2, V2)) ? V1 : V2;
}

//----------------------------------------------------------------------------------
float AccumulatedHorizonOcclusion(float2 deltaUV,
    float2 uv0,
    float3 P,
    float numSteps,
    float randstep,
    float3 dPdu,
    float3 dPdv)
{
    // Randomize starting point within the first sample distance
    //float2 uv = uv0 + snap_uv_offset(randstep * deltaUV);
    float2 uv = uv0 + deltaUV;

    // Snap increments to pixels to avoid disparities between xy 
    // and z sample locations and sample along a line
    //deltaUV = snap_uv_offset(deltaUV);

    // Compute tangent vector using the tangent plane
    float3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;

    //float tanH = biased_tangent(T);
    float tanH = tangent(T);
    float sinH = tanH / sqrt(1.0f + tanH * tanH);

    float ao = 0;
    for (float j = 1; j <= numSteps; ++j) {
        uv += deltaUV;
        float3 S = ScreenToViewPos(uv);

        // Ignore any samples outside the radius of influence
        float3 D = S - P;
        float d2 = dot(D, D);
        if (d2 < Radius.y) {
            float tanS = tangent(P, S);

            [branch]
            if (tanS > tanH) {
                // Accumulate AO between the horizon and the sample
                float sinS = tanS / sqrt(1.0f + tanS * tanS);
                float r = sqrt(d2) * Radius.z;
               // ao += falloff(r) * (sinS - sinH);
                ao += (sinS - sinH);

                // Update the current horizon angle
                tanH = tanS;
                sinH = sinS;
            }
        }
    }

    return ao;
}

[RootSignature(HBAO_RootSig)]
[numthreads(8, 8, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    if (dispatchThreadId.x >= (uint)ScreenSize.x || dispatchThreadId.y >= (uint)ScreenSize.y)
        return;
    
    float2 PixelCenter = dispatchThreadId.xy + float2(0.5, 0.5);
    float2 UV = PixelCenter * ScreenSize.zw;
	float3 P = ScreenToViewPos(UV);

    // Project radius from eye space to texture space
    float2 StepSize = 0.5f * Radius.x * FocalLen.xy / P.z;

    // Early out if the projected radius is smaller than 1 pixel
    float NumSteps = min(MAX_STEPS, min(StepSize.x * ScreenSize.x, StepSize.y * ScreenSize.y));
    StepSize = StepSize / (float)(NumSteps + 1);

    float3 L, R, T, B;
    L = ScreenToViewPos(UV + float2(-ScreenSize.z, 0));
    R = ScreenToViewPos(UV + float2( ScreenSize.z, 0));
    T = ScreenToViewPos(UV + float2(0,  ScreenSize.w));
    B = ScreenToViewPos(UV + float2(0, -ScreenSize.w));
    float3 N = normalize(cross(R - L, T - B));
    float4 TangentPlane = float4(N, dot(P, N));

    float3 dPdU = MinDiff(P, R, L);
    float3 dPdV = MinDiff(P, T, B) * (ScreenSize.y * ScreenSize.z);

    float AO = 0.f;
	[unroll]
    for (int d = 0; d < MAX_DIR; d++)
    {
        float2 Dir = float2(cos(d * ANGLE_STEP), sin(d * ANGLE_STEP));
        float2 DeltaUV = float2(Dir.x - Dir.y, Dir.x + Dir.y) * StepSize.xy;

        AO += AccumulatedHorizonOcclusion(DeltaUV, UV, P, NumSteps, 0.f, dPdU, dPdV);
    }

    AO = 1.0 - AO / MAX_DIR;


	AOResult[dispatchThreadId.xy] = AO;
}
