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

#define P2Cell_RootSig \
	"CBV(b0) ," \
	"CBV(b1) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=2))" 

StructuredBuffer<float3> Positions : register(t0);
RWStructuredBuffer<uint> NumInCell : register(u0);
RWStructuredBuffer<uint> CellParticles : register(u1);

[RootSignature(P2Cell_RootSig)]
[numthreads(128, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint TotalParticles = Dim.w; 
    uint Index = dispatchThreadId.x;   
	if (Index >= TotalParticles)
		return;

    float3 Pos = Positions[Index];
    const float cell_size_inv = P1.w;

    int3 CellIndex3 = int3((Pos - BMin.xyz) * cell_size_inv);
    uint CellIndex = GetCellHash(CellIndex3, Dim.xyz);

    uint OriginNum;
    InterlockedAdd(NumInCell[CellIndex].x, 1, OriginNum);

    if (OriginNum < MaxParticleInCell)
    {
        uint CellParticleIndex = CellIndex * MaxParticleInCell + OriginNum;
        CellParticles[CellParticleIndex] = Index;
    }
}
