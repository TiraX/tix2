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
struct IndirectCommand
{
	uint2 cbvAddress;
	uint4 drawArguments;
};

StructuredBuffer<FTileVisibleInfo> TileVisibles		: register(t0);	// Tile visible infos
StructuredBuffer<FPrimitiveMetaInfo> PrimitiveInfos	: register(t1); // Primitive info , include tile index
StructuredBuffer<IndirectCommand> InputCommands		: register(t2);	// All commands buffer
RWStructuredBuffer<IndirectCommand> OutputCommands	: register(u0);	// Cull result, if this tile is visible

#define threadBlockSize 16

[RootSignature(CopyVisibleCommands_RootSig)]
[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint Index = groupId.x * threadBlockSize + threadIDInGroup.x;
	if (Index < Info.x)
	{
	//	// Test if this tile intersect with frustum
	//	// 1, test BBox
	//	if (TileInfos[Index].MinEdge.x <= BBoxMax.x &&
	//		TileInfos[Index].MinEdge.y <= BBoxMax.y &&
	//		TileInfos[Index].MinEdge.z <= BBoxMax.z &&
	//		TileInfos[Index].MaxEdge.x >= BBoxMin.x &&
	//		TileInfos[Index].MaxEdge.y >= BBoxMin.y &&
	//		TileInfos[Index].MaxEdge.z >= BBoxMin.z)
	//	{
	//		// 2, test frustum
	//		if (IntersectPlaneBBox(Planes[0], TileInfos[Index].MinEdge, TileInfos[Index].MaxEdge) &&
	//			IntersectPlaneBBox(Planes[1], TileInfos[Index].MinEdge, TileInfos[Index].MaxEdge) &&
	//			IntersectPlaneBBox(Planes[2], TileInfos[Index].MinEdge, TileInfos[Index].MaxEdge) &&
	//			IntersectPlaneBBox(Planes[3], TileInfos[Index].MinEdge, TileInfos[Index].MaxEdge) &&
	//			IntersectPlaneBBox(Planes[4], TileInfos[Index].MinEdge, TileInfos[Index].MaxEdge) &&
	//			IntersectPlaneBBox(Planes[5], TileInfos[Index].MinEdge, TileInfos[Index].MaxEdge))
	//		{
	//			VisibleInfo[Index].Visible = 1;
	//		}
	//		else
	//		{
	//			VisibleInfo[Index].Visible = 0;
	//		}
	//	}
	//	else
	//	{
	//		VisibleInfo[Index].Visible = 0;
	//	}
	}
}
