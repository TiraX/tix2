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

#define Lambda_RootSig \
	"CBV(b0) ," \
	"CBV(b1) ," \
    "DescriptorTable(SRV(t0, numDescriptors=3), UAV(u0, numDescriptors=1))" 

StructuredBuffer<float3> Positions : register(t0);
StructuredBuffer<uint> NeighborNum : register(t1);
StructuredBuffer<uint> NeighborParticles : register(t2);

RWStructuredBuffer<float> Lambdas : register(u0);

[RootSignature(Lambda_RootSig)]
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

    float3 Grad = float3(0, 0, 0);
    float SumGradSq = 0.f;
    float Density = 0.f;
    for (int i = 0 ; i < NumNb ; i ++)
    {
        int NbIndex = NeighborParticles[NbParticleOffset + i];
        float3 NbPos = Positions[NbIndex];

        float3 Dir = Pos - NbPos;
        float s = length(Dir);
        Dir /= s;
        float3 NbGrad = spiky_gradient(Dir, s, h, h3_inv) * m_by_rho;
        Grad += NbGrad;
        SumGradSq += dot(NbGrad, NbGrad);
        Density += poly6_value(s, h, h2, h3_inv);
    }

    SumGradSq += dot(Grad, Grad);
    float DensityContraint = max(Density * m_by_rho, 0.f);
    Lambdas[Index] = (-DensityContraint) / (SumGradSq + epsilon);
}
