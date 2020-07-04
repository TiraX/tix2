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
static const int MAX_STEPS = 35;
static const float STEP_LENGTH = 2.f;
static const int RAND_TEX_SIZE = 64 - 1;

cbuffer FInfoUniform : register(b0)
{
    float4 ScreenSize;  // xy = Size; zw = InvSize;
    float4 FocalLen;    // xy = FocalLen; zw = InvFocalLen;
    float4 Radius;      // x = radius; y = radius^2; z = 1.0/radius
    float4 Falloff;     // x = scale, y = bias
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

float FalloffFn(float disSQ)
{
    return 2.0 * saturate(disSQ * Falloff.x + Falloff.y);
}

float SearchForLargestAngle(float2 inUV, float2 deltaUV, float3 ViewSpacePosition, float3 ViewDir)
{
    float BiggestAngle = -1.0f;
    //float PrevAngle = -1.0f;

    float2 UV = inUV;

    [unroll]
    for (int i = 1; i <= MAX_STEPS; i++)
    {
        UV += deltaUV;

        float3 SampVS = ScreenToViewPos(UV);
        float3 DiffV = (SampVS - ViewSpacePosition);
        float distSQ = dot(DiffV, DiffV);

        [branch]
            if (distSQ < Radius.y)
            {
                float ooDist = rsqrt(distSQ);
                float cosh = ooDist * dot(DiffV, ViewDir);

                float falloff = 0.5;// FalloffFn(distSQ);

                cosh = cosh - falloff;
                BiggestAngle = max(BiggestAngle, cosh);//UpdateMaxAngle(inUV, BiggestAngle, cosh, PrevAngle);
                //PrevAngle = cosh;
            }

    }


    return BiggestAngle;
}


// Given a screen space Axis this will search for the min and max angles
float2 SearchAxisForAngles(float2 UV, float3 ScreenDir, float3 ViewDir, float3 ViewSpacePos, float2 DeltaUV, float Offset)
{
    //float2 DeltaUV = Scale * float2(GTAOParams[2].z * ScreenDir.x, GTAOParams[2].w * ScreenDir.y);
    //DeltaUV.y *= -1;

    float Ang1 = SearchForLargestAngle(UV + (DeltaUV * Offset), DeltaUV, ViewSpacePos, ViewDir);
    float Ang2 = SearchForLargestAngle(UV - (DeltaUV * Offset), -DeltaUV, ViewSpacePos, ViewDir);
    return float2(Ang1, Ang2);
}

float ComputeInnerIntegral(float2 Angles, float3 ScreenDir, float3 ViewDir, float3 ViewSpaceNormal, float SceneDepth)
{

    float3 PlaneNormal = normalize(cross(ScreenDir, ViewDir));
    float3 Perp = cross(ViewDir, PlaneNormal);
    float3 ProjNormal = ViewSpaceNormal - PlaneNormal * dot(ViewSpaceNormal, PlaneNormal);

    float LenProjNormal = length(ProjNormal);
    float RecipMag = 1.0f / (LenProjNormal + 1e-6);
    float CosAng = dot(ProjNormal, Perp) * RecipMag;
    float Gamma = acosFast(CosAng) - (PI * 0.5);
    float CosGamma = dot(ProjNormal, ViewDir) * RecipMag;
    float SinGamma = CosAng * -2.0f;


    Angles.x = Gamma + max(-Angles.x - Gamma, -((PI * 0.5)));
    Angles.y = Gamma + min(Angles.y - Gamma, ((PI * 0.5)));


    LenProjNormal = 1;




    float AO = (LenProjNormal * 0.25 *
        ((Angles.x * SinGamma + CosGamma - cos((2.0 * Angles.x) - Gamma)) +
            (Angles.y * SinGamma + CosGamma - cos((2.0 * Angles.y) - Gamma))));
    AO = saturate(AO / (PI * 0.5));








    //float Mul = ScreenSpaceAOParams[4].x;
    //float Add = ScreenSpaceAOParams[4].y;
    //AO = lerp(AO, 1, saturate(SceneDepth * Mul + Add));

    return AO;
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

    float3 ViewDir = -normalize(P);

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

    // load random (cos(alpha), sin(alpha), jitter)
    float3 RandDir = RandomTex.Load(int3((int)(dispatchThreadId.x & RAND_TEX_SIZE), (int)(dispatchThreadId.y & RAND_TEX_SIZE), 0)).xyz;
    RandDir = RandDir * 2.f - 1.f;

    float AO = 0.f;

    float3 ScreenDir = float3(1, 0, 0);// float3(RandDir.x, RandDir.y, 0.0);
    float2 DeltaUV = float2(ScreenDir.x * RandDir.x - ScreenDir.y * RandDir.y, ScreenDir.x * RandDir.y + ScreenDir.y * RandDir.x) * StepSize.xy;
    float2 Angles = SearchAxisForAngles(UV, ScreenDir, ViewDir, P, DeltaUV, 0.0);// RandDir.z);

    Angles.x = acosFast(Angles.x);
    Angles.y = acosFast(Angles.y);

    AO += ComputeInnerIntegral(Angles, ScreenDir, ViewDir, N, P.z);


	AOResult[dispatchThreadId.xy] = AO;
}
