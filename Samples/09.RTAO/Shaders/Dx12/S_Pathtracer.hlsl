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
    float4 CamU, CamV, CamW;
};
typedef BuiltInTriangleIntersectionAttributes MyAttributes;

RaytracingAccelerationStructure SceneAccelerationStruct : register(t0);
RWTexture2D<float4> RTColor : register(u0);

[shader("raygeneration")]
void MyRayGenShader()
{
    uint2 CurPixel = DispatchRaysIndex().xy;
    uint2 TotalPixels = DispatchRaysDimensions().xy;
    float2 PixelCenter = (CurPixel + float2(0.5f, 0.5f)) / TotalPixels;
    float2 Ndc = float2(2, -2) * PixelCenter + float2(-1, 1);
    float3 RayDir = Ndc.x * CamU.xyz + Ndc.y * CamV.xyz + CamW.xyz;

    RayDesc Ray;
    Ray.Origin = CamPos.xyz;
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
    data.RayColor = float3(1, 1, 0);
}
