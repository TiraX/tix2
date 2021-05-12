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
    "DescriptorTable(SRV(t0, numDescriptors=4), UAV(u0, numDescriptors=1))" 

StructuredBuffer<uint> NeighborNum : register(t0);
StructuredBuffer<uint> NeighborParticles : register(t1);
StructuredBuffer<float> Lambdas : register(t2);
StructuredBuffer<float3> Positions : register(t3);

RWStructuredBuffer<float3> DeltaPosition : register(u0);

[RootSignature(DeltaPos_RootSig)]
[numthreads(128, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint TotalParticles = Dim.w; 
    uint Index = dispatchThreadId.x;   
	if (Index >= TotalParticles)
		return;

    float3 Pos = Positions[Index];

    const float mass = P0.x;
    const float epsilon = P0.y;
    const float m_by_rho = P0.z;
    const float dt = P0.w;
    
    const float h = P1.x;
    const float h2 = P1.y;
    const float h3_inv = P1.z;
    const float cell_size_inv = P1.w;

    const int NumNb = NeighborNum[Index];
    const int NbParticleOffset = Index * MaxNeighbors;
    
    const float Lambda = Lambdas[Index];

    float3 DeltaPos = float3(0, 0, 0);
    for (int i = 0 ; i < NumNb ; i ++)
    {
        int NbIndex = NeighborParticles[NbParticleOffset + i];

        float NbLambda = Lambdas[NbIndex];

        float3 NbPos = Positions[NbIndex];
        float3 Dir = Pos - NbPos;
        float s = max(length(Dir), 1e-6);
        Dir /= s;

        DeltaPos += spiky_gradient(Dir, s, h, h3_inv) * (Lambda + NbLambda);
    }

    DeltaPos *= m_by_rho;
    DeltaPosition[Index] = DeltaPos;
}
