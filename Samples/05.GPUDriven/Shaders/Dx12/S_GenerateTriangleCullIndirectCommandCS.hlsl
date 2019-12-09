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

#define TriangleCullIndirectCommand_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=2), UAV(u0, numDescriptors=1))"

cbuffer FClusterCount : register(b0)
{
	uint4 ClusterCount;
};

struct TriangleCullDispatchCommand
{
	uint IndexOffset;
	uint InstanceIndex;
	uint2 VertexDataView;
	uint2 IndexDataView;
	uint3 ThreadGroupCount;
};

struct DrawIndirectCommand
{
	uint4 VertexBufferView;
	uint4 IndexBufferView;
	uint4 DrawArguments0;	// x=IndexCountPerInstance,y=InstanceCount,z=StartIndexLocation,w=BaseVertexLocation
	uint DrawArguments1;	// x=StartInstanceLocation
};

struct FClusterMetaInfo
{
	// x = instance global index
	// y = cluster global index
	// z = draw command index
	// w = cluster local index
	uint4 Info;
};

StructuredBuffer<FClusterMetaInfo> VisibleClusters : register(t0);
StructuredBuffer<DrawIndirectCommand> DrawCommands : register(t1);	// Draw commands buffer, get vertex and index address from it
AppendStructuredBuffer<TriangleCullDispatchCommand> OutputCommands : register(u0);	// Cull result, if this tile is visible

[RootSignature(TriangleCullIndirectCommand_RootSig)]
[numthreads(64, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint ClusterIndex = dispatchThreadId.x;// groupId.x * threadBlockSize + threadIDInGroup.x;
	if (ClusterIndex >= ClusterCount.x)
		return;
	
	uint InstanceGlobalIndex = VisibleClusters[ClusterIndex].Info.x;
	uint DrawCommandIndex = VisibleClusters[ClusterIndex].Info.z;
	uint ClusterLocalIndex = VisibleClusters[ClusterIndex].Info.w;

	DrawIndirectCommand DrawCommand = DrawCommands[DrawCommandIndex];

	TriangleCullDispatchCommand Command;
	Command.IndexOffset = ClusterLocalIndex * 128;
	Command.InstanceIndex = InstanceGlobalIndex;
	Command.VertexDataView = DrawCommand.VertexBufferView.xy;
	Command.IndexDataView = DrawCommand.IndexBufferView.xy;
	Command.ThreadGroupCount.x = 128;
	Command.ThreadGroupCount.y = 1;
	Command.ThreadGroupCount.z = 1;
	OutputCommands.Append(Command);
}
