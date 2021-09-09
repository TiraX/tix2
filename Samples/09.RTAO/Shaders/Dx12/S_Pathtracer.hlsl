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
    float3 RayColor;
};

GlobalRootSignature MyGlobalRootSignature =
{
    "DescriptorTable(UAV(u0), SRV(t0)),"    // Output texture, AS
    "CBV(b0),"                              // Scene constants
};

cbuffer RayGenData : register(b0)
{
    float4 CamPos;
    float4x4 ProjectionToWorld;
};
typedef BuiltInTriangleIntersectionAttributes MyAttributes;

RaytracingAccelerationStructure SceneAccelerationStruct : register(t0);
RWTexture2D<float4> RTColor : register(u0);

// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid.
inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a ray.
    float4 world = mul(float4(screenPos, 0, 1), ProjectionToWorld);

    world.xyz /= world.w;
    origin = CamPos.xyz;
    direction = normalize(world.xyz - origin);
}

[shader("raygeneration")]
void MyRayGenShader()
{
    uint2 CurPixel = DispatchRaysIndex().xy;
    float3 Origin, Direction;
    GenerateCameraRay(CurPixel, Origin, Direction);

    RayDesc Ray;
    Ray.Origin = Origin;
    Ray.Direction = Direction;
    Ray.TMin = 0.f;
    Ray.TMax = 1e+38f;

    SimpleRayPayload Payload = {float3(0, 0, 0)};

    TraceRay(SceneAccelerationStruct, RAY_FLAG_NONE, 0xFF, 0, 1, 0, Ray, Payload); 
    RTColor[CurPixel] = float4(Payload.RayColor, 1.f);
}

[shader("miss")]
void RayMiss(inout SimpleRayPayload data)
{
    data.RayColor = float3(0.1, 0, 0);
}

[shader("anyhit")]
void RayAnyHit(inout SimpleRayPayload data, in MyAttributes attribs)
{
    data.RayColor = float3(attribs.barycentrics, 0.6);
}

[shader("closesthit")]
void RayClosestHit(inout SimpleRayPayload data, in MyAttributes attribs)
{
    data.RayColor = float3(attribs.barycentrics, 0.2);
}
