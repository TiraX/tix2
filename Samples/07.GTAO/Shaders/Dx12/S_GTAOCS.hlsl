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
static const float PI_HALF = PI * 0.5f;
static const int MAX_DIR = 16;
static const float MAX_DIR_INV = 1.0 / float(MAX_DIR);
static const int MAX_STEPS = 12;
static const float STEP_LENGTH = 2.f;
static const float ANGLE_STEP = PI * 2.f / MAX_DIR;
static const int RAND_TEX_SIZE = 64 - 1;

#define SSAO_LIMIT 100
#define SSAO_SAMPLES 4
#define SSAO_RADIUS 2.5
#define SSAO_FALLOFF 1.5
#define SSAO_THICKNESSMIX 0.2
#define SSAO_MAX_STRIDE 32

cbuffer FInfoUniform : register(b0)
{
    float4 ScreenSize;  // xy = Size; zw = InvSize;
    float4 FocalLen;    // xy = FocalLen; zw = InvFocalLen;
    float4 Radius;      // x = radius; y = radius^2; z = 1.0/radius
};

Texture2D<float4> SceneNormal : register(t0);
Texture2D<float> SceneDepth : register(t1);
Texture2D<float4> RandomTex : register(t2);

RWTexture2D<float4> AOResult : register(u0);

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

// Same cost as acosFast + 1 FR
// Same error
// input [-1, 1] and output [-PI/2, PI/2]
float asinFast(float x)
{
    return (0.5 * PI) - acosFast(x);
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
    float3 V,
    float numSteps,
    float randstep)
{
    // Randomize starting point within the first sample distance
    //float2 uv = uv0 + snap_uv_offset(randstep * deltaUV);
    float2 uv = uv0 +randstep * deltaUV;

    float CosValue = -1;
    for (float j = 1; j <= numSteps; ++j) {
        uv += deltaUV;
        float3 S = ScreenToViewPos(uv);

        // Ignore any samples outside the radius of influence
        float3 D = (S - P);
        float D2 = dot(D, D);
        if (D2 < Radius.y)
        {
            float Current = dot(normalize(D), V);

            float Falloff = clamp((Radius.x - length(D)) / SSAO_FALLOFF, 0.f, 1.f);
            if (Current > CosValue)
                CosValue = Current;// lerp(CosValue, Current, Falloff);
            //CosValue = lerp(CosValue, Current, SSAO_THICKNESSMIX * Falloff);
        }
    }

    return CosValue;
}

float2 SearchAxisForAngles(float2 deltaUV,
    float2 uv0,
    float3 P,
    float3 V,
    float numSteps,
    float randstep)
{
    float h1 = SearchForLargestAngle(deltaUV, uv0, P, V, numSteps, randstep);
    float h2 = SearchForLargestAngle(-deltaUV, uv0, P, V, numSteps, randstep);

    return float2(h1, h2);
}

float ComputeInnerIntegral(float2 Angles, float NoV, float n)
{
    float cosN = NoV;
    float sinN = sqrt(1.f - NoV * NoV);
    float a1 = -cos(Angles.x * 2.f - n);
    float a2 = -cos(Angles.y * 2.f - n);

    //return (a1 + cosN + Angles.x * 2.f * sinN + a2 + cosN + Angles.y * 2.f * sinN) * 0.25f;
    return 2.f - cos(Angles.x) - cos(Angles.y);
}

float IntegrateArc(float h1, float h2, float n)
{
    float cosN = cos(n);
    float sinN = sin(n);
    return 0.25 * (-cos(2.0 * h1 - n) + cosN + 2.0 * h1 * sinN - cos(2.0 * h2 - n) + cosN + 2.0 * h2 * sinN);
    //return 2.f - cos(h1) - cos(h2);
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
    float3 V = -normalize(P);

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

    float Stride = 6;// min((1.0 / length(P)) * SSAO_LIMIT, SSAO_MAX_STRIDE);
    float2 dirMult = ScreenSize.zw * Stride;

    float3 dPdU = MinDiff(P, R, L);
    float3 dPdV = MinDiff(P, T, B) * (ScreenSize.y * ScreenSize.z);

    // load random (cos(alpha), sin(alpha), jitter)
    float3 RandDir = RandomTex.Load(int3((int)(dispatchThreadId.x & RAND_TEX_SIZE), (int)(dispatchThreadId.y & RAND_TEX_SIZE), 0)).xyz;
    RandDir = RandDir * 2.f - 1.f;

    float3 debug = float3(2,2,2);
    float AO = 0.f;
    [unroll]
    for (int d = 0; d < MAX_DIR; d++)
    {
        float2 Dir = float2(cos(d * ANGLE_STEP), sin(d * ANGLE_STEP));
        float2 DeltaUV = float2(Dir.x * RandDir.x - Dir.y * RandDir.y, Dir.x * RandDir.y + Dir.y * RandDir.x) * StepSize.xy;
    
        // TODO , here do NOT need to sample depth in ScreenToViewPos
        float3 toDir = ScreenToViewPos(UV + DeltaUV);
        float3 planeNormal = normalize(cross(V, -toDir));
        float3 projectedNormal = N - planeNormal * dot(N, planeNormal);

        float3 projectedDir = normalize(normalize(toDir) + V);
        float n = acosFast(dot(-projectedDir, normalize(projectedNormal))) - PI_HALF;

        float2 CosValues = SearchAxisForAngles(DeltaUV, UV, P, V, NumSteps, RandDir.z);

        float h1a = -acosFast(CosValues.x);
        float h2a = acosFast(CosValues.y);

        float h1 = n + max(h1a - n, -PI_HALF);
        float h2 = n + min(h2a - n, PI_HALF);

        debug.x = h1a * 180.0f / PI;
        debug.y = h2a * 180.0f / PI;
        debug.z = n * 180.0f / PI;
        AO += lerp(1.f, IntegrateArc(h1, h2, n), length(projectedNormal));
    }

    AO = AO / MAX_DIR;

    AOResult[dispatchThreadId.xy] = float4(AO, AO, AO, 1);
}
