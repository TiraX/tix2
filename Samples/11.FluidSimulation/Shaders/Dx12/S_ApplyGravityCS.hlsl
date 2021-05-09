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
#include "S_PbfDef.hlsli"

#define ApplyGravity_RootSig \
	"CBV(b0) ," \
	"CBV(b1) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=2))" 

StructuredBuffer<float3> Velocities : register(t0);
RWStructuredBuffer<float3> Positions : register(u0);
RWStructuredBuffer<float3> PosOld : register(u1);

[RootSignature(ApplyGravity_RootSig)]
[numthreads(128, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint TotalParticles = Dim.w; 
    uint Index = dispatchThreadId.x;   
	if (Index >= TotalParticles)
		return;

    float3 Pos = Positions[Index];
    const float dt = P0.w;

    // Save old position
    PosOld[Index] = Pos;
    
    // Apply gravity
    float3 Vel = Velocities[Index];
    Vel += GRAVITY * dt;
    Pos += Vel.xyz * dt;

    // Boundary check
    BoundaryCheck(Pos, BMin.xyz, BMax.xyz);

    Positions[Index] = Pos;
}
