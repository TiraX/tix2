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

#define Sort_RootSig \
	"CBV(b0) ," \
	"CBV(b1) ," \
    "DescriptorTable(SRV(t0, numDescriptors=5), UAV(u0, numDescriptors=2))" 

StructuredBuffer<float3> Positions : register(t0);
StructuredBuffer<float3> Velocities : register(t1);
StructuredBuffer<uint> NumInCell : register(t2);
StructuredBuffer<uint> CellParticleOffsets : register(t3);
StructuredBuffer<uint> CellParticles : register(t4);

RWStructuredBuffer<float3> SortedPositions : register(u0);
RWStructuredBuffer<float3> SortedVelocities : register(u1);

[RootSignature(Sort_RootSig)]
[numthreads(128, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint TotalCells = Dim.x * Dim.y * Dim.z; 
    uint CellIndex = dispatchThreadId.x;   
	if (CellIndex >= TotalCells)
		return;
    
    uint Num = NumInCell[CellIndex];
    uint Offset = CellParticleOffsets[CellIndex];
    for (uint i = 0 ; i < Num ; i ++)
    {
        uint SrcIndex = CellParticles[CellIndex * MaxParticleInCell + i];
        uint DestIndex = Offset + i;

        SortedPositions[DestIndex] = Positions[SrcIndex];
        SortedVelocities[DestIndex] = Velocities[SrcIndex];
    }
}
