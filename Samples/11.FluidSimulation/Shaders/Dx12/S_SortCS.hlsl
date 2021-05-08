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
    "DescriptorTable(SRV(t0, numDescriptors=4), UAV(u0, numDescriptors=1))" 

StructuredBuffer<FParticle> Particles : register(t0);
StructuredBuffer<uint> NumInCell : register(t1);
StructuredBuffer<uint> CellParticleOffsets : register(t2);
StructuredBuffer<uint> CellParticles : register(t3);

RWStructuredBuffer<FParticle> SortedParticles : register(u0);

[RootSignature(Sort_RootSig)]
[numthreads(128, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint TotalParticles = Dim.w; 
    uint Index = dispatchThreadId.x;   
	if (Index >= TotalParticles)
		return;
    
    uint Num = NumInCell[Index];
    uint Offset = CellParticleOffsets[Index];
    for (uint i = 0 ; i < Num ; i ++)
    {
        uint SrcIndex = CellParticles[Offset + i];
        uint DestIndex = Offset + i;

        SortedParticles[DestIndex] = Particles[SrcIndex];
    }
}
