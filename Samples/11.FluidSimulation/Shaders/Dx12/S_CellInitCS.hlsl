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

#define CellInit_RootSig \
	"CBV(b0) ," \
	"CBV(b1) ," \
    "DescriptorTable(UAV(u0, numDescriptors=2))" 

RWStructuredBuffer<uint> NumInCell : register(u0);
RWStructuredBuffer<uint> CellParticleOffsets : register(u1);

[RootSignature(CellInit_RootSig)]
[numthreads(128, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint TotalCells = Dim.x * Dim.y * Dim.z; 
    uint Index = dispatchThreadId.x;   
	if (Index >= TotalCells)
		return;

    NumInCell[Index] = 0;
    CellParticleOffsets[Index] = 0;
}
