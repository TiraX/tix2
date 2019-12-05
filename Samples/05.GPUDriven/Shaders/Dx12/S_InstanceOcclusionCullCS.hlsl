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

#define InstanceOcclusionCull_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=5), UAV(u0, numDescriptors=2))," \
    "StaticSampler(s0, addressU = TEXTURE_ADDRESS_CLAMP, " \
                      "addressV = TEXTURE_ADDRESS_CLAMP, " \
                      "addressW = TEXTURE_ADDRESS_CLAMP, " \
                        "filter = FILTER_MIN_MAG_MIP_POINT)"

cbuffer FOcclusionInfo : register(b0)
{
	float4x4 ViewProjection;
	uint4 RTSize;	// xy = size, z = max_mip_level
};

struct FVisibleInfo
{
	uint Visible;
};

struct FBBox
{
	float4 MinEdge;
	float4 MaxEdge;
};

struct FInstanceMetaInfo
{
	// x = primitive index
	// y = cluster index begin
	// z = cluster count
	// w = primitive was loaded.
	uint4 Info;
};

struct FClusterMetaInfo
{
	// x = instance global index
	// y = cluster global index
	// z = draw command index
	// w = cluster local index
	uint4 Info;
};

struct FInstanceTransform
{
	float4 ins_transition;
	float4 ins_transform0;
	float4 ins_transform1;
	float4 ins_transform2;
};

StructuredBuffer<FBBox> PrimitiveBBoxes : register(t0);
StructuredBuffer<FInstanceMetaInfo> InstanceMetaInfo : register(t1);
StructuredBuffer<FInstanceTransform> InstanceData : register(t2);
StructuredBuffer<FVisibleInfo> FrustumCullResult : register(t3);
Texture2D<float> HiZTexture : register(t4);

RWStructuredBuffer<FVisibleInfo> VisibleInfo : register(u0);	// Cull result, if this instance is visible
AppendStructuredBuffer<uint4> ClustersQueue : register(u1);	// Clusters to be culled

SamplerState PointSampler : register(s0);

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

[RootSignature(InstanceOcclusionCull_RootSig)]
[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint InstanceIndex = dispatchThreadId.x;// groupId.x * threadBlockSize + threadIDInGroup.x;
	uint Result = 0;
	if (InstanceMetaInfo[InstanceIndex].Info.w > 0 &&
		FrustumCullResult[InstanceIndex].Visible > 0)
	{
		// This tile intersect view frustum, need to cull instances one by one
		uint PrimitiveIndex = InstanceMetaInfo[InstanceIndex].Info.x;

		// Transform primitive bbox
		float4 MinEdge = PrimitiveBBoxes[PrimitiveIndex].MinEdge;
		float4 MaxEdge = PrimitiveBBoxes[PrimitiveIndex].MaxEdge;
		TransformBBox(InstanceData[InstanceIndex], MinEdge, MaxEdge);

		float3 BBoxMin = MinEdge.xyz;
		float3 BBoxMax = MaxEdge.xyz;
		float3 BBoxSize = BBoxMax - BBoxMin;

		float3 BBoxCorners[] = 
		{
			BBoxMin.xyz,
			BBoxMin.xyz + float3(BBoxSize.x,0,0),
			BBoxMin.xyz + float3(0, BBoxSize.y,0),
			BBoxMin.xyz + float3(0, 0, BBoxSize.z),
			BBoxMin.xyz + float3(BBoxSize.xy,0),
			BBoxMin.xyz + float3(0, BBoxSize.yz),
			BBoxMin.xyz + float3(BBoxSize.x, 0, BBoxSize.z),
			BBoxMin.xyz + BBoxSize.xyz
		};

		float minZ = 1;
		float2 minXY = 1;
		float2 maxXY = 0;

		[unroll]
		for (int i = 0; i < 8; i++)
		{
			//transform world space aaBox to NDC
			float4 ClipPos = mul(float4(BBoxCorners[i], 1), ViewProjection);

			ClipPos.z = max(ClipPos.z, 0);

			ClipPos.xyz = ClipPos.xyz / ClipPos.w;

			ClipPos.xy = clamp(ClipPos.xy, -1, 1);
			ClipPos.xy = ClipPos.xy * float2(0.5, -0.5) + float2(0.5, 0.5);

			minXY = min(ClipPos.xy, minXY);
			maxXY = max(ClipPos.xy, maxXY);

			minZ = saturate(min(minZ, ClipPos.z));
		}

		float4 BoxUVs = float4(minXY, maxXY);

		// Calculate hi-Z buffer mip
		int2 Size = int2((maxXY - minXY) * RTSize.xy);
		float Mip = ceil(log2(max(Size.x, Size.y)));

		uint MaxMipLevel = RTSize.z;
		Mip = clamp(Mip, 0, MaxMipLevel);

		// Texel footprint for the lower (finer-grained) level
		float  level_lower = max(Mip - 1, 0);
		float2 scale = exp2(-level_lower);
		float2 a = floor(BoxUVs.xy*scale);
		float2 b = ceil(BoxUVs.zw*scale);
		float2 dims = b - a;

		// Use the lower level if we only touch <= 2 texels in both dimensions
		if (dims.x <= 2 && dims.y <= 2)
			Mip = level_lower;

		//load depths from high z buffer
		float4 Depth = 
		{
			HiZTexture.SampleLevel(PointSampler, BoxUVs.xy, Mip),
			HiZTexture.SampleLevel(PointSampler, BoxUVs.zy, Mip),
			HiZTexture.SampleLevel(PointSampler, BoxUVs.xw, Mip),
			HiZTexture.SampleLevel(PointSampler, BoxUVs.zw, Mip)
		};

		//find the max depth
		float maxDepth = max(max(max(Depth.x, Depth.y), Depth.z), Depth.w);

		if (minZ <= maxDepth)
		{
			Result = 1;

			// Copy visible clusters for cluster culling
			uint4 ClusterInfo;
			ClusterInfo.x = InstanceIndex;	// Instance index
			ClusterInfo.z = PrimitiveIndex; // Primitive index
			uint ClusterIndex = InstanceMetaInfo[InstanceIndex].Info.y;
			uint ClusterCount = InstanceMetaInfo[InstanceIndex].Info.z;
			for (uint i = 0; i < ClusterCount; ++i)
			{
				ClusterInfo.y = ClusterIndex + i;	// Cluster index;
				ClusterInfo.w = i;
				ClustersQueue.Append(ClusterInfo);
			}
		}
	}
	VisibleInfo[InstanceIndex].Visible = Result;
}
