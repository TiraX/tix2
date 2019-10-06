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

#define CopyVisibleCommands_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=3), UAV(u0)),"

cbuffer CopyParams : register(b0)
{
	// x: The number of commands to be processed.
	uint4 Info;
};
struct FTileVisibleInfo
{
	uint Visible;
};
struct FPrimitiveMetaInfo
{
	uint4 Info;
};
struct VBView
{
	uint2 Address;
	uint SizeInBytes;
	uint StrideInBytes;
};
struct IBView
{
	uint2 Address;
	uint SizeInBytes;
	uint Format;
};
struct IndirectCommand
{
	uint4 VertexBufferView;
	uint4 InstanceBufferView;
	uint4 IndexBufferView;
	uint4 DrawArguments0;
	uint DrawArguments1;
};

StructuredBuffer<FTileVisibleInfo> TileVisibles		: register(t0);	// Tile visible infos
StructuredBuffer<FPrimitiveMetaInfo> PrimitiveInfos	: register(t1); // Primitive info , include tile index
StructuredBuffer<IndirectCommand> InputCommands		: register(t2);	// All commands buffer
AppendStructuredBuffer<IndirectCommand> OutputCommands	: register(u0);	// Cull result, if this tile is visible

#define threadBlockSize 128

[RootSignature(CopyVisibleCommands_RootSig)]
[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint Index = groupId.x * threadBlockSize + threadIDInGroup.x;
	if (Index < Info.x)
	{
		uint TileIndex = PrimitiveInfos[Index].Info.x;
		if (TileVisibles[TileIndex].Visible > 0)
		{
			OutputCommands.Append(InputCommands[Index]);
		}
	}
}
