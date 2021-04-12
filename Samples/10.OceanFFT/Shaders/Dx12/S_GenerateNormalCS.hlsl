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

#define GenerateNormal_RS \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=1))" 

cbuffer FInfoUniform : register(b0)
{
    float4 Info;    // x = Size
};

Texture2D<float4> OceanDisplacement : register(t0);
RWTexture2D<float4> NormalTexture : register(u0);

[RootSignature(GenerateNormal_RS)]
[numthreads(32, 32, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    int2 Size = int2(Info.xy);
    int2 Pos = dispatchThreadId.xy;

    const float D   = 0.03f;
    float3 Center   = OceanDisplacement.Load(int3(Pos, 0)).xyz;
    int2 UpPos      = Pos + int2(0, -1);
    UpPos.y         = UpPos.y < 0 ? UpPos.y + Size.y : UpPos.y;
    float3 Up       = OceanDisplacement.Load(int3(UpPos, 0)).xyz + float3(0, -D, 0);
    int2 LeftPos    = Pos + int2(-1, 0);
    LeftPos.x       = LeftPos.x < 0 ? LeftPos.x + Size.x : LeftPos.x;
    float3 Left     = OceanDisplacement.Load(int3(LeftPos, 0)).xyz + float3(-D, 0, 0);
    float3 N0       = normalize(cross(normalize(Left - Center), normalize(Up - Center)));
    
    uint2 DownPos   = uint2(Pos + int2(0, 1)) % Size;
    float3 Down     = OceanDisplacement.Load(int3(DownPos, 0)).xyz + float3(0, D, 0);
    uint2 RightPos  = uint2(Pos + int2(1, 0)) % Size;
    float3 Right    = OceanDisplacement.Load(int3(RightPos, 0)).xyz + float3(D, 0, 0);
    float3 N1       = normalize(cross(normalize(Right - Center), normalize(Down - Center)));

    float3 Result   = normalize(N0 + N1) * 0.5 + 0.5;


    NormalTexture[Pos] = float4(Result, 1.0);
}
