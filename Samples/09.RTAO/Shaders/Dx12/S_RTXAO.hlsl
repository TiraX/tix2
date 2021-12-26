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

struct SimpleRayPayload
{
    float AO;
};

GlobalRootSignature MyGlobalRootSignature =
{
    "DescriptorTable(UAV(u0), SRV(t0, numDescriptors=3)),"    // Output texture, AS, ScenePosition, SceneNormal
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
RWTexture2D<float4> RTAO : register(u0);

#define RAY_LEN 2.0
#define NUM_RAYS 1

[shader("raygeneration")]
void MyRayGenShader()
{
    uint2 CurPixel = DispatchRaysIndex().xy;
    float3 Origin, Direction;
    //GenerateCameraRay(CurPixel, Origin, Direction);

    float3 WorldPosition = ScenePosition[CurPixel].xyz;
    float3 WorldNormal = SceneNormal[CurPixel].xyz;

    RayDesc Ray;
    Ray.Origin = WorldPosition;
    Ray.TMin = 0.001f;
    Ray.TMax = RAY_LEN;

    float ao = NUM_RAYS;
    for (int i = 0; i < NUM_RAYS; i++)
    {
        SimpleRayPayload Payload;
        Payload.AO = 0;

        Ray.Direction = GetRandomDir(WorldNormal);

        uint Flag = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH |
            RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;
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
