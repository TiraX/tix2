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

#define NbSearch_RootSig \
	"CBV(b0) ," \
	"CBV(b1) ," \
    "DescriptorTable(SRV(t0, numDescriptors=3), UAV(u0, numDescriptors=2))" 

StructuredBuffer<uint> NumInCell : register(t0);
StructuredBuffer<uint> CellParticleOffsets : register(t1);
StructuredBuffer<float3> Positions : register(t2);

RWStructuredBuffer<uint> NeighborNum : register(u0);
RWStructuredBuffer<uint> NeighborParticles : register(u1);

[RootSignature(NbSearch_RootSig)]
[numthreads(128, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint TotalParticles = Dim.w; 
    uint Index = dispatchThreadId.x;   
	if (Index >= TotalParticles)
		return;

    float3 Pos = Positions[Index];

    const float h2 = P1.y;
    const float cell_size_inv = P1.w;

    // Find neighbours
    const int3 CellIndexOffsets [ 27 ] = 
    {
        int3(-1,-1,-1),
        int3(-1,-1, 0),
        int3(-1,-1, 1),
        int3(-1, 0,-1),
        int3(-1, 0, 0),
        int3(-1, 0, 1),
        int3(-1, 1,-1),
        int3(-1, 1, 0),
        int3(-1, 1, 1),

        int3(0,-1,-1),
        int3(0,-1, 0),
        int3(0,-1, 1),
        int3(0, 0,-1),
        int3(0, 0, 0),
        int3(0, 0, 1),
        int3(0, 1,-1),
        int3(0, 1, 0),
        int3(0, 1, 1),

        int3(1,-1,-1),
        int3(1,-1, 0),
        int3(1,-1, 1),
        int3(1, 0,-1),
        int3(1, 0, 0),
        int3(1, 0, 1),
        int3(1, 1,-1),
        int3(1, 1, 0),
        int3(1, 1, 1),
    };
    uint NumNb = 0;
    int3 CellIndex3 = int3((Pos.xyz - BMin.xyz) * cell_size_inv);
    const int NbParticleOffset = Index * MaxNeighbors;
    for (int cell = 0; cell < 27; ++cell) 
	{
        int3 NbCellIndex3 = CellIndex3 + CellIndexOffsets[cell];
        if (NbCellIndex3.x >= 0 && NbCellIndex3.x < Dim.x &&
            NbCellIndex3.y >= 0 && NbCellIndex3.y < Dim.y &&
            NbCellIndex3.z >= 0 && NbCellIndex3.z < Dim.z)
        {
            uint NbCellIndex = GetCellHash(NbCellIndex3, Dim.xyz);
            uint NumNeighborsInCell = NumInCell[NbCellIndex];
            uint NbParticleCellOffset = CellParticleOffsets[NbCellIndex];

            for (uint i = 0 ; i < NumNeighborsInCell ; i ++)
            {
                uint NbIndex = NbParticleCellOffset + i;
                if (NbIndex != Index)
                {
                    float3 NbPos = Positions[NbIndex];
                    float3 Dir = Pos - NbPos;
                    float LengthSq = dot(Dir, Dir);
                    if (LengthSq < h2 && NumNb < MaxNeighbors)
                    {
                        NeighborParticles[NbParticleOffset + NumNb] = NbIndex;
                        NumNb ++;
                    }
                }
            }
        }
    }
    NeighborNum[Index] = NumNb;
}
