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

cbuffer FInfoUniform : register(b0)
{
    float4 ScreenSize; // xy = Size; zw = InvSize;
    float4 InvFocalLen;
	float4 ViewDir;
};

Texture2D<float4> SceneNormal : register(t0);
Texture2D<float> SceneDepth : register(t1);

RWTexture2D<float> AOResult : register(u0);

SamplerState PointSampler : register(s0);

static const float PI = 3.14159f;
static const int MAX_DIR = 6;
static const float MAX_DIR_INV = 1.0 / float(MAX_DIR);
static const int MAX_STEPS = 6;
static const float STEP_LENGTH = 2.f;
static const float ANGLE_STEP = PI * 2.f / MAX_DIR;


float3 ScreenToViewPos(float2 UV, float Depth)
{
	UV = UV * float2(2.0, 2.0) - float2(1.0, 1.0);
	return float3(UV * InvFocalLen.xy * Depth, Depth);
}

float3 ScreenToViewPos(float2 UV)
{
    float Depth = SceneDepth.SampleLevel(PointSampler, UV, 0);
	return ScreenToViewPos(UV, Depth);
}

[RootSignature(HBAO_RootSig)]
[numthreads(8, 8, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    if (dispatchThreadId.x >= (uint)ScreenSize.x || dispatchThreadId.y >= (uint)ScreenSize.y)
        return;
    
    float2 PixelCenter = dispatchThreadId.xy + float2(0.5, 0.5);
    float2 uv = PixelCenter * ScreenSize.zw;
	float3 P = ScreenToViewPos(uv);
    float3 WorldNormal = normalize(SceneNormal.SampleLevel(PointSampler, uv, 0).xyz);
	float CosT = dot(ViewDir.xyz, WorldNormal);
	float SinT = sqrt(1.0 - CosT * CosT);
	float AO = 0.f;
    
	[unroll]
    for (int d = 0; d < MAX_DIR; d++)
    {
        float2 Dir = float2(cos(ANGLE_STEP * d), sin(ANGLE_STEP * d));
        
        // Find max angle in this direction
        float MaxDepth = -999.f;
		float SinH = 0.f;
        [unroll]
        for (int s = 0; s < MAX_STEPS; ++s)
        {
            float2 UV = (PixelCenter + Dir * STEP_LENGTH * s) * ScreenSize.zw;
            float Depth = SceneDepth.SampleLevel(PointSampler, UV, 0);

			if (Depth > MaxDepth)
			{
				float3 S = ScreenToViewPos(uv, Depth);
				MaxDepth = Depth;

				float3 H = S - P;
				SinH = H.z / sqrt(dot(H, H));
			}
        }
		AO += SinH - SinT;
    }
	AO *= MAX_DIR_INV;

	AOResult[dispatchThreadId.xy] = AO;
}
