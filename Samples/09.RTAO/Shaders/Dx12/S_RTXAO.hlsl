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

#include "RandomNumberGenerator.hlsli"

struct SimpleRayPayload
{
    float AO;
};

GlobalRootSignature MyGlobalRootSignature =
{
    "DescriptorTable(UAV(u0), SRV(t0, numDescriptors=4)),"    // Output texture, AS, ScenePosition, SceneNormal, RandDirTex
};

//LocalRootSignature MyLocalRootSignature =
//{
//    "RootConstants( num32BitConstants = 1, b1 )"           // Cube constants        
//};

TriangleHitGroup MyHitGroup =
{
    "RayAnyHit",       // AnyHit
    "",   // ClosestHit
};

//SubobjectToExportsAssociation  MyLocalRootSignatureAssociation =
//{
//    "MyLocalRootSignature",  // subobject name
//    "MyHitGroup"             // export association 
//};

RaytracingShaderConfig  MyShaderConfig =
{
    16, // max payload size
    8   // max attribute size
};

RaytracingPipelineConfig MyPipelineConfig =
{
    1 // max trace recursion depth
};

//cbuffer RayGenData : register(b0)
//{
//    float4 CamPos;
//    float4x4 ProjectionToWorld;
//};
typedef BuiltInTriangleIntersectionAttributes MyAttributes;

RaytracingAccelerationStructure SceneAccelerationStruct : register(t0);
Texture2D<float4> ScenePosition : register(t1);
Texture2D<float4> SceneNormal : register(t2);
Texture2D<float4> RandDirTex : register(t3);
RWTexture2D<float4> RTAO : register(u0);

#define RAY_LEN 0.15
#define NUM_RAYS 64

float3 GetRandomDir(in float3 WorldPos, in float3 WorldNormal, int Iteration)
{
    // Calculate coordinate system for the hemisphere.
    float3 u, v, w;
    w = WorldNormal;

    // Get a vector that's not parallel to w.
    float3 right = 0.3f * w + float3(-0.72f, 0.56f, -0.34f);
    v = normalize(cross(w, right));
    u = cross(v, w);

    uint Seed = Iteration * RNG::hash(WorldPos);
    uint RNGState = RNG::SeedThread(Seed);

    // Find a random sample
    uint SampleLocation = RNG::Random(RNGState, 0, 4095);
    int2 SampleLocation2;
    SampleLocation2.x = SampleLocation % 64;
    SampleLocation2.y = SampleLocation / 64;
    float3 RandDir = normalize(RandDirTex[SampleLocation2].xyz * 2.0 - 1.0);
    return normalize(u * RandDir.x + v * RandDir.y + w * RandDir.z);
}

[shader("raygeneration")]
void MyRayGenShader()
{
    uint2 CurPixel = DispatchRaysIndex().xy;
    float3 Origin, Direction;
    //GenerateCameraRay(CurPixel, Origin, Direction);

    float3 WorldPosition = ScenePosition[CurPixel].xyz;
    float3 WorldNormal = normalize(SceneNormal[CurPixel].xyz * 2.0 - float3(1.0, 1.0, 1.0));

    RayDesc Ray;
    Ray.Origin = WorldPosition + WorldNormal * 0.1f;
    Ray.TMin = 0;
    Ray.TMax = RAY_LEN;

    uint Flag = RAY_FLAG_CULL_FRONT_FACING_TRIANGLES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;

    SimpleRayPayload Payload;
    float ao = 0;
    for (int i = 0; i < NUM_RAYS; i++)
    {
        Payload.AO = 0;

        Ray.Direction = GetRandomDir(WorldPosition + float3(CurPixel.xy, 0), WorldNormal, i);

        TraceRay(SceneAccelerationStruct, Flag, 0xFF, 0, 1, 0, Ray, Payload);
        ao += Payload.AO;
    }
    ao /= float(NUM_RAYS);
    RTAO[CurPixel] = float4(ao, ao, ao, 1.f);
}

[shader("miss")]
void RayMiss(inout SimpleRayPayload data)
{
    data.AO = 1;
}

[shader("anyhit")]
void RayAnyHit(inout SimpleRayPayload data, in MyAttributes attribs)
{
    data.AO = 0;
}

//[shader("closesthit")]
//void RayClosestHit(inout SimpleRayPayload data, in MyAttributes attribs)
//{
//    data.AO = 0;
//}
