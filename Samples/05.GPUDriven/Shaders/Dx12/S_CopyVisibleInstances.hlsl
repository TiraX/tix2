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
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=3), UAV(u0)),"

cbuffer CopyParams : register(b0)
{
	// x: The number of commands to be processed.
	uint4 Info;
};
struct FVisibleInfo
{
	uint Visible;
};
struct FInstanceMetaInfo
{
	// x = primitive index
	// y = scene tile index
	uint4 Info;
};
struct IndirectCommand
{
	uint4 VertexBufferView;
	uint4 InstanceBufferView;
	uint4 IndexBufferView;
	uint4 DrawArguments0;
	uint DrawArguments1;
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
	if (InstanceMetaInfos[InstanceIndex].Info.w > 0.0)
	{
		uint PrimitiveIndex = InstanceMetaInfos[InstanceIndex].Info.x;

		IndirectCommand Command = InputCommands[PrimitiveIndex];

	}
	//if (Index < Info.x)
	//{
	//	uint TileIndex = PrimitiveInfos[Index].Info.x;
	//	if (TileVisibles[TileIndex].Visible > 0)
	//	{
	//		OutputCommands.Append(InputCommands[Index]);
	//	}
	//}
}
