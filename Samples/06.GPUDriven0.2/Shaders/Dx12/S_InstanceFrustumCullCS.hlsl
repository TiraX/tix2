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

#define InstanceFrustumCull_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=4), UAV(u0, numDescriptors=1)),"


cbuffer FFrustum : register(b0)
{
	float4 BBoxMin;
	float4 BBoxMax;
	float4 Planes[6];
};

struct FBBox
{
	float4 MinEdge;
	float4 MaxEdge;
};

// PUT shared struct in a single header
struct FInstanceMetaInfo
{
	// Info.x = scene mesh index this instance link to, in FScene::SceneMeshes order, to access scene mesh bbox
	// Info.y = if this primitive is loaded. 1 = loaded; 0 = loading
	// Info.z = cluster index begin
	// Info.w = cluster count
	uint4 Info;
};

struct FInstanceTransform
{
	float4 ins_transition;
	float4 ins_transform0;
	float4 ins_transform1;
	float4 ins_transform2;
};

struct FDrawInstanceCommand
{
	//uint32 IndexCountPerInstance;
	//uint32 InstanceCount;
	//uint32 StartIndexLocation;
	//uint32 BaseVertexLocation;
	//uint32 StartInstanceLocation;
	uint4 Params;
	uint Param;
};


StructuredBuffer<FBBox> SceneMeshBBox : register(t0);
StructuredBuffer<FInstanceMetaInfo> InstanceMetaInfo : register(t1);
StructuredBuffer<FInstanceTransform> InstanceData : register(t2);
StructuredBuffer<FDrawInstanceCommand> DrawCommandBuffer : register(t3);

AppendStructuredBuffer<FDrawInstanceCommand> OutputDrawCommandBuffer : register(u0);

inline bool IntersectPlaneBBox(float4 Plane, float4 MinEdge, float4 MaxEdge)
{
	float3 P;
	P.x = Plane.x >= 0.0 ? MinEdge.x : MaxEdge.x;
	P.y = Plane.y >= 0.0 ? MinEdge.y : MaxEdge.y;
	P.z = Plane.z >= 0.0 ? MinEdge.z : MaxEdge.z;

	float dis = dot(Plane.xyz, P) + Plane.w;
	return dis <= 0.0;
}

inline void TransformBBox(FInstanceTransform Trans, inout float4 MinEdge, inout float4 MaxEdge)
{
	float3 VecMin = MinEdge.xyz;
	float3 VecMax = MaxEdge.xyz;

	float3 Origin = (VecMax + VecMin) * 0.5;
	float3 Extent = (VecMax - VecMin) * 0.5;

	float3 NewOrigin = Origin.xxx * Trans.ins_transform0.xyz;
	NewOrigin = Origin.yyy * Trans.ins_transform1.xyz + NewOrigin;
	NewOrigin = Origin.zzz * Trans.ins_transform2.xyz + NewOrigin;
	NewOrigin += Trans.ins_transition.xyz;

	float3 NewExtent = abs(Extent.xxx * Trans.ins_transform0.xyz);
	NewExtent += abs(Extent.yyy * Trans.ins_transform1.xyz);
	NewExtent += abs(Extent.zzz * Trans.ins_transform2.xyz);

	MinEdge.xyz = NewOrigin - NewExtent;
	MaxEdge.xyz = NewOrigin + NewExtent;
}

#define threadBlockSize 128

[RootSignature(InstanceFrustumCull_RootSig)]
[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint DrawCmdIndex = dispatchThreadId.x;

	if (InstanceMetaInfo[DrawCmdIndex].Info.y > 0)	// Test loaded primitives
	{
		uint InstanceIndex = DrawCommandBuffer[DrawCmdIndex].Param;
		uint MeshIndex = InstanceMetaInfo[DrawCmdIndex].Info.x;
		// Transform primitive bbox
		float4 MinEdge = SceneMeshBBox[MeshIndex].MinEdge;
		float4 MaxEdge = SceneMeshBBox[MeshIndex].MaxEdge;
		TransformBBox(InstanceData[InstanceIndex], MinEdge, MaxEdge);

		if (IntersectPlaneBBox(Planes[0], MinEdge, MaxEdge) &&
			IntersectPlaneBBox(Planes[1], MinEdge, MaxEdge) &&
			IntersectPlaneBBox(Planes[2], MinEdge, MaxEdge) &&
			IntersectPlaneBBox(Planes[3], MinEdge, MaxEdge) &&
			IntersectPlaneBBox(Planes[4], MinEdge, MaxEdge) &&
			IntersectPlaneBBox(Planes[5], MinEdge, MaxEdge))
		{
			OutputDrawCommandBuffer.Append(DrawCommandBuffer[DrawCmdIndex]);
		}
	}
}
