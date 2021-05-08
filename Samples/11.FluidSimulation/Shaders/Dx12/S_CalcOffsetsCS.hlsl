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
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=1))" 

StructuredBuffer<uint> NumInCell : register(t0);
RWStructuredBuffer<uint> CellParticleOffsets : register(u0);

[RootSignature(CellInit_RootSig)]
[numthreads(1, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    CellParticleOffsets[0] = 0;
    for (int i = 1 ; i < Dim.w; i ++)
    {
        CellParticleOffsets[i] = CellParticleOffsets[i - 1] + NumInCell[i - 1];
    }
}
