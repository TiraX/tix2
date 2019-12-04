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
	"CBV(b0) ," \
    "DescriptorTable(UAV(u0)),"

cbuffer FClusterCount : register(b0)
{
	uint4 ClusterCount;
};

struct IndirectCommand
{
	uint3 ThreadGroupCount;
};

AppendStructuredBuffer<IndirectCommand> OutputCommands : register(u0);	// Cull result, if this tile is visible

[RootSignature(ClusterCullIndirectCommand_RootSig)]
[numthreads(1, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	IndirectCommand Command;
	Command.ThreadGroupCount.x = (ClusterCount + 127) / 128;
	Command.ThreadGroupCount.y = 1;
	Command.ThreadGroupCount.z = 1;
	OutputCommands.Append(Command);
	//OutputCommands[0].ThreadGroupCount.x = (ClusterCount + 127) / 128;
	//OutputCommands[0].ThreadGroupCount.y = 1;
	//OutputCommands[0].ThreadGroupCount.z = 1;
}
