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

#define GTAO_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=3), UAV(u0, numDescriptors=1))," \
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
static const int RAND_TEX_SIZE = 64 - 1;

cbuffer FInfoUniform : register(b0)
{
    float4 ScreenSize;  // xy = Size; zw = InvSize;
    float4 FocalLen;    // xy = FocalLen; zw = InvFocalLen;
    float4 Radius;      // x = radius; y = radius^2; z = 1.0/radius
};

Texture2D<float4> SceneNormal : register(t0);
Texture2D<float> SceneDepth : register(t1);
Texture2D<float4> RandomTex : register(t2);

RWTexture2D<float> AOResult : register(u0);

SamplerState PointSampler : register(s0);

// Relative error : ~3.4% over full
// Precise format : ~small float
// 2 ALU
float rsqrtFast(float x)
{
    int i = asint(x);
    i = 0x5f3759df - (i >> 1);
    return asfloat(i);
}

// Relative error : < 0.7% over full
// Precise format : ~small float
// 1 ALU
float sqrtFast(float x)
{
    int i = asint(x);
    i = 0x1FBD1DF5 + (i >> 1);
    return asfloat(i);
}

// max absolute error 9.0x10^-3
// Eberly's polynomial degree 1 - respect bounds
// 4 VGPR, 12 FR (8 FR, 1 QR), 1 scalar
// input [-1, 1] and output [0, PI]
float acosFast(float inX)
{
    float x = abs(inX);
    float res = -0.156583f * x + (0.5 * PI);
    res *= sqrtFast(1.0f - x);
    return (inX >= 0) ? res : PI - res;
}

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
float SearchForLargestAngle(float2 deltaUV,
    float2 uv0,
    float3 P,
    float numSteps,
    float randstep,
    float3 dPdu,
    float3 dPdv)
{
    // Randomize starting point within the first sample distance
    //float2 uv = uv0 + snap_uv_offset(randstep * deltaUV);
    float2 uv = uv0 + randstep * deltaUV;

    // Snap increments to pixels to avoid disparities between xy 
    // and z sample locations and sample along a line
    //deltaUV = snap_uv_offset(deltaUV);

    // Compute tangent vector using the tangent plane
    float3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;

    //float tanH = biased_tangent(T);
    float tanH = tangent(T);
    float sinH = tanH / sqrt(1.0f + tanH * tanH);

    float angle = 0;
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
                float cosS = rsqrt(1.f + tanS * tanS);
                angle = acosFast(cosS);
                //// Accumulate AO between the horizon and the sample
                //float sinS = tanS / sqrt(1.0f + tanS * tanS);
                //float r = sqrt(d2) * Radius.z;
                //// ao += falloff(r) * (sinS - sinH);
                //ao += (sinS - sinH);

                // Update the current horizon angle
                tanH = tanS;
            }
        }
    }

    return angle;
}

float2 SearchAxisForAngles(float2 deltaUV,
    float2 uv0,
    float3 P,
    float numSteps,
    float randstep,
    float3 dPdu,
    float3 dPdv)
{
    float h1 = SearchForLargestAngle(deltaUV, uv0, P, numSteps, randstep, dPdu, dPdv);
    float h2 = -SearchForLargestAngle(-deltaUV, uv0, P, numSteps, randstep, dPdu, dPdv);

    return float2(h1, h2);
}

float ComputeInnerIntegral(float2 Angles, float NoV, float n)
{
    float cosN = NoV;
    float sinN = sqrt(1.f - NoV * NoV);
    float a1 = -cos(Angles.x * 2.f - n);
    float a2 = -cos(Angles.y * 2.f - n);

    return (a1 + cosN + Angles.x * 2.f * sinN + a2 + cosN + Angles.y * 2.f * sinN) * 0.25f;
}

[RootSignature(GTAO_RootSig)]
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
    R = ScreenToViewPos(UV + float2(ScreenSize.z, 0));
    T = ScreenToViewPos(UV + float2(0, ScreenSize.w));
    B = ScreenToViewPos(UV + float2(0, -ScreenSize.w));
    float3 N = normalize(cross(R - L, T - B));
    float4 TangentPlane = float4(N, dot(P, N));

    float3 dPdU = MinDiff(P, R, L);
    float3 dPdV = MinDiff(P, T, B) * (ScreenSize.y * ScreenSize.z);

    // load random (cos(alpha), sin(alpha), jitter)
    float3 RandDir = RandomTex.Load(int3((int)(dispatchThreadId.x & RAND_TEX_SIZE), (int)(dispatchThreadId.y & RAND_TEX_SIZE), 0)).xyz;
    RandDir = RandDir * 2.f - 1.f;

    float NoV = dot(N, float3(0, 0, 1));
    float n = acosFast(NoV);

    float AO = 0.f;
    [unroll]
    for (int d = 0; d < MAX_DIR; d++)
    {
        float2 Dir = float2(cos(d * ANGLE_STEP), sin(d * ANGLE_STEP));
        float2 DeltaUV = float2(Dir.x * RandDir.x - Dir.y * RandDir.y, Dir.x * RandDir.y + Dir.y * RandDir.x) * StepSize.xy;

        float2 Angles = SearchAxisForAngles(DeltaUV, UV, P, NumSteps, RandDir.z, dPdU, dPdV);

        //Angles.x = acosFast(Angles.x);
        //Angles.y = acosFast(Angles.y);

        AO += ComputeInnerIntegral(Angles, NoV, n);
    }

    AO = 1.f - AO / MAX_DIR;


    AOResult[dispatchThreadId.xy] = AO;
}
