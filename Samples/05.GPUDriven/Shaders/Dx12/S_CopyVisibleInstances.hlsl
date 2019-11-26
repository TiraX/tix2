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

#define CopyVisibleInstances_RootSig \
    "DescriptorTable(SRV(t0, numDescriptors=3), UAV(u0)),"

struct FVisibleInfo
{
	uint Visible;
};
struct FInstanceMetaInfo
{
	// x = primitive index
	uint4 Info;
};
struct IndirectCommand
{
	uint4 VertexBufferView;
	uint4 IndexBufferView;
	uint4 DrawArguments0;	// x=IndexCountPerInstance,y=InstanceCount,z=StartIndexLocation,w=BaseVertexLocation
	uint DrawArguments1;	// x=StartInstanceLocation
};

StructuredBuffer<FVisibleInfo> InstanceVisibleInfos : register(t0);
StructuredBuffer<FInstanceMetaInfo> InstanceMetaInfos : register(t1); // Primitive info , include tile index
StructuredBuffer<IndirectCommand> InputCommands : register(t2);	// All commands buffer
AppendStructuredBuffer<IndirectCommand> OutputCommands : register(u0);	// Cull result, if this tile is visible

#define threadBlockSize 128

[RootSignature(CopyVisibleInstances_RootSig)]
[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint InstanceIndex = groupId.x * threadBlockSize + threadIDInGroup.x;
	if (InstanceMetaInfos[InstanceIndex].Info.w > 0 &&
		InstanceVisibleInfos[InstanceIndex].Visible > 0)
	//if (InstanceMetaInfos[InstanceIndex].Info.w > 0 )
		{
		uint PrimitiveIndex = InstanceMetaInfos[InstanceIndex].Info.x;

		IndirectCommand Command = InputCommands[PrimitiveIndex];
		if (Command.VertexBufferView.x > 0)
		{
			Command.DrawArguments0.y = 1;	// Change instance count to 1
			Command.DrawArguments1.x = InstanceIndex;

			OutputCommands.Append(Command);
		}
	}
}
