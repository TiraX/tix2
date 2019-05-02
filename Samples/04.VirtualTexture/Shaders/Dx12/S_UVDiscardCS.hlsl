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

#define UVDiscard_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0)),"

cbuffer RootConstants : register(b0)
{
	float4 Info;	// x, y groups
};

struct UVBuffer
{
	float4 color;
};
StructuredBuffer<UVBuffer> inputUVs : register(t0);	// SRV: Indirect commands
RWStructuredBuffer<UVBuffer> outputUVs : register(u0);	// UAV: Processed indirect commands

#define threadBlockSize 8

[RootSignature(UVDiscard_RootSig)]
[numthreads(threadBlockSize, threadBlockSize, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID)
{
	if (threadIDInGroup.x == 0 && threadIDInGroup.y == 0)
	{
		uint GroupW = uint(Info.x);
		uint input_index = (groupId.y * threadBlockSize + threadIDInGroup.y) * threadBlockSize * GroupW + groupId.x * threadBlockSize + threadIDInGroup.x;
		uint output_index = groupId.y * GroupW + groupId.x;
		outputUVs[output_index] = inputUVs[input_index];
	}
}
