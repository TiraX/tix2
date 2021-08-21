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

cbuffer RayGenData 
{
    float3 CamPos;
    float3 CamU, CamV, CamW;
};
typedef BuiltInTriangleIntersectionAttributes MyAttributes;

RWTexture2D<float4> RTColor;
RaytracingAccelerationStructure SceneAccelerationStruct;

[shader("raygeneration")]
void MyRayGenShader()
{
    uint2 CurPixel = DispatchRaysIndex().xy;
    uint2 TotalPixels = DispatchRaysDimensions().xy;
    float2 PixelCenter = (CurPixel + float2(0.5f, 0.5f)) / TotalPixels;
    float2 Ndc = float2(2, -2) * PixelCenter + float2(-1, 1);
    float3 RayDir = Ndc.x * CamU + Ndc.y * CamV + CamW;

    RayDesc Ray;
    Ray.Origin = CamPos;
    Ray.Direction = normalize(RayDir);
    Ray.TMin = 0.f;
    Ray.TMax = 1e+38f;

    SimpleRayPayload Payload = {float3(0, 0, 0)};

    TraceRay(SceneAccelerationStruct, RAY_FLAG_NONE, 0xFF, 0, 1, 0, Ray, Payload); 
    RTColor[CurPixel] = float4(Payload.RayColor, 1.f);
}

[shader("miss")]
void RayMiss(inout SimpleRayPayload data)
{
    data.RayColor = float3(0, 0, 1);
}

[shader("closesthit")]
void RayClosestHit(inout SimpleRayPayload data, in MyAttributes attribs)
{
    data.RayColor = float3(1, 0, 0);
}
