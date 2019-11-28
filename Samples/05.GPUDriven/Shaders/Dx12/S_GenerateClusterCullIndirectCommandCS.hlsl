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

#define ClusterCullIndirectCommand_RootSig \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0)),"

struct IndirectCommand
{
	uint3 ThreadGroupCount;
};

StructuredBuffer<uint2> ClustersLeft : register(t0);
RWStructuredBuffer<IndirectCommand> OutputCommands : register(u0);	// Cull result, if this tile is visible

[RootSignature(ClusterCullIndirectCommand_RootSig)]
[numthreads(1, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint numStructs, stride;
	ClustersLeft.GetDimensions(numStructs, stride);

	OutputCommands[0].ThreadGroupCount.x = (numStructs + 127) / 128;
	OutputCommands[0].ThreadGroupCount.y = 1;
	OutputCommands[0].ThreadGroupCount.z = 1;
}
