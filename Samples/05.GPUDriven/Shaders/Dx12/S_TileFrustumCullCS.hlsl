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

#define TileFrustumCull_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0)),"

struct FTileInfo
{
	float4 MinEdge;
	float4 MaxEdge;
};

struct FTileVisibleInfo
{
	uint Visible;
};

cbuffer FFrustum : register(b0)
{
	float4 BBoxMin;
	float4 BBoxMax;
	float4 Planes[6];
};

StructuredBuffer<FTileInfo> TileInfos : register(t0);	// Tile infos, like bbox
RWStructuredBuffer<FTileVisibleInfo> VisibleInfo : register(u0);	// Cull result, if this tile is visible


inline bool IntersectPlaneBBox(float4 Plane, float4 MinEdge, float4 MaxEdge)
{
	float3 P;
	P.x = Plane.x >= 0.0 ? MinEdge.x : MaxEdge.x;
	P.y = Plane.y >= 0.0 ? MinEdge.y : MaxEdge.y;
	P.z = Plane.z >= 0.0 ? MinEdge.z : MaxEdge.z;

	float dis = dot(Plane.xyz, P) + Plane.w;
	return dis <= 0.0;
}

#define threadBlockSize 16

[RootSignature(TileFrustumCull_RootSig)]
[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint Index = groupId.x * threadBlockSize + threadIDInGroup.x;
	if (TileInfos[Index].MinEdge.w > 0.0)
	{
		// Test if this tile intersect with frustum
		// 1, test BBox
		if (TileInfos[Index].MinEdge.x <= BBoxMax.x &&
			TileInfos[Index].MinEdge.y <= BBoxMax.y &&
			TileInfos[Index].MinEdge.z <= BBoxMax.z &&
			TileInfos[Index].MaxEdge.x >= BBoxMin.x &&
			TileInfos[Index].MaxEdge.y >= BBoxMin.y &&
			TileInfos[Index].MaxEdge.z >= BBoxMin.z)
		{
			// 2, test frustum
			if (IntersectPlaneBBox(Planes[0], TileInfos[Index].MinEdge, TileInfos[Index].MaxEdge) &&
				IntersectPlaneBBox(Planes[1], TileInfos[Index].MinEdge, TileInfos[Index].MaxEdge) &&
				IntersectPlaneBBox(Planes[2], TileInfos[Index].MinEdge, TileInfos[Index].MaxEdge) &&
				IntersectPlaneBBox(Planes[3], TileInfos[Index].MinEdge, TileInfos[Index].MaxEdge) &&
				IntersectPlaneBBox(Planes[4], TileInfos[Index].MinEdge, TileInfos[Index].MaxEdge) &&
				IntersectPlaneBBox(Planes[5], TileInfos[Index].MinEdge, TileInfos[Index].MaxEdge))
			{
				VisibleInfo[Index].Visible = 1;
			}
			else
			{
				VisibleInfo[Index].Visible = 0;
			}
		}
		else
		{
			VisibleInfo[Index].Visible = 0;
		}
	}
}
