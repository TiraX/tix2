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
    "DescriptorTable(SRV(t0, numDescriptors=4), UAV(u0, numDescriptors=4)),"


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

struct FInstanceMetaInfo
{
	// x = draw call index
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


StructuredBuffer<FBBox> PrimitiveBBoxes : register(t0);
StructuredBuffer<FInstanceMetaInfo> InstanceMetaInfo : register(t1);
StructuredBuffer<FInstanceTransform> InstanceData : register(t2);
StructuredBuffer<FDrawInstanceCommand> DrawCommandBuffer : register(t3);

RWStructuredBuffer<FInstanceTransform> CompactInstanceData : register(u0);
RWStructuredBuffer<FDrawInstanceCommand> OutputDrawCommandBuffer : register(u1);
RWStructuredBuffer<uint> VisibleInstanceIndex : register(u2);
RWStructuredBuffer<uint4> VisibleInstanceCount : register(u3);	// x = Visible instance count; y = Dispatch thread group count;

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
	uint InstanceIndex = dispatchThreadId.x;// groupId.x * threadBlockSize + threadIDInGroup.x;
	uint PrimitiveIndex = InstanceMetaInfo[InstanceIndex].Info.x;

	if (InstanceMetaInfo[InstanceIndex].Info.w > 0)	// Test loaded primitives
	{
		// Transform primitive bbox
		float4 MinEdge = PrimitiveBBoxes[PrimitiveIndex].MinEdge;
		float4 MaxEdge = PrimitiveBBoxes[PrimitiveIndex].MaxEdge;
		TransformBBox(InstanceData[InstanceIndex], MinEdge, MaxEdge);

		if (IntersectPlaneBBox(Planes[0], MinEdge, MaxEdge) &&
			IntersectPlaneBBox(Planes[1], MinEdge, MaxEdge) &&
			IntersectPlaneBBox(Planes[2], MinEdge, MaxEdge) &&
			IntersectPlaneBBox(Planes[3], MinEdge, MaxEdge) &&
			IntersectPlaneBBox(Planes[4], MinEdge, MaxEdge) &&
			IntersectPlaneBBox(Planes[5], MinEdge, MaxEdge))
		{
			// Increate visible instances count
			uint CurrentInstanceCount;
			InterlockedAdd(OutputDrawCommandBuffer[PrimitiveIndex].Params.y, 1, CurrentInstanceCount);

			// Copy instance data to compaced position
			CompactInstanceData[DrawCommandBuffer[PrimitiveIndex].Param + CurrentInstanceCount] = InstanceData[InstanceIndex];

			// Modify Draw command
			OutputDrawCommandBuffer[PrimitiveIndex].Params.x = DrawCommandBuffer[PrimitiveIndex].Params.x;
			//OutputDrawCommandBuffer[PrimitiveIndex].Params.x = DrawCommandBuffer[PrimitiveIndex].Params.x;
			OutputDrawCommandBuffer[PrimitiveIndex].Params.z = DrawCommandBuffer[PrimitiveIndex].Params.z;
			OutputDrawCommandBuffer[PrimitiveIndex].Params.w = DrawCommandBuffer[PrimitiveIndex].Params.w;
			OutputDrawCommandBuffer[PrimitiveIndex].Param = DrawCommandBuffer[PrimitiveIndex].Param;

			// Add visible instance to array
			uint TotalInstanceCount;
			InterlockedAdd(VisibleInstanceCount[0].x, 1, TotalInstanceCount);
			//VisibleInstanceCount[0].y = uint(TotalInstanceCount + 127) / 128;
			VisibleInstanceIndex[TotalInstanceCount] = InstanceIndex;
		}
	}
}
