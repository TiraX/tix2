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

#define DeltaPos_RootSig \
	"CBV(b0) ," \
	"CBV(b1) ," \
    "DescriptorTable(SRV(t0, numDescriptors=3), UAV(u0, numDescriptors=1))" 

StructuredBuffer<float3> PosOld : register(t0);
RWStructuredBuffer<FParticle> Particles : register(u0);

[RootSignature(DeltaPos_RootSig)]
[numthreads(128, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint TotalParticles = Dim.w; 
    uint Index = dispatchThreadId.x;   
	if (Index >= TotalParticles)
		return;

    FParticle Particle = Particles[Index];

    const float mass = P0.x;
    const float epsilon = P0.y;
    const float m_by_rho = P0.z;
    const float dt = P0.w;
    
    // Boundary check
    BoundaryCheck(Particle.Pos.xyz, BMin.xyz, BMax.xyz);

    // Update Velocity
    Particle.Vel.xyz = (Particle.Pos.xyz - PosOld[Index]) / dt;

    Particles[Index] = Particle;
}
