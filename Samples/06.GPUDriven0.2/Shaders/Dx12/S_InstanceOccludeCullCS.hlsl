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

#define InstanceOccludeCull_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=7), UAV(u0, numDescriptors=4))," \
	"StaticSampler(s0, addressU = TEXTURE_ADDRESS_CLAMP, " \
						"addressV = TEXTURE_ADDRESS_CLAMP, " \
						"addressW = TEXTURE_ADDRESS_CLAMP, " \
						"filter = FILTER_MIN_MAG_MIP_POINT)"


cbuffer FOcclusionInfo : register(b0)
{
	float4x4 ViewProjection;
	uint4 RTSize;	// xy = size, z = max_mip_level
};

struct FBBox
{
	float4 MinEdge;
	float4 MaxEdge;
};

// PUT shared struct in a single header
struct FInstanceMetaInfo
{
	// Info1.x = mesh bbox index
	// Info1.y = draw call index
	// Info1.w = loaded
	uint4 Info1;
	// Info2.x = cluster index begin
	// Info2.y = cluster count
	uint4 Info2;
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
StructuredBuffer<uint> VisibleInstanceIndex : register(t4);
StructuredBuffer<uint4> VisibleInstanceCount : register(t5);	// x = Visible instance count; y = Dispatch thread group count;
Texture2D<float> HiZTexture : register(t6);

RWStructuredBuffer<FInstanceTransform> CompactInstanceData : register(u0);	// For render test
RWStructuredBuffer<FDrawInstanceCommand> OutputDrawCommandBuffer : register(u1);	// For render test
RWStructuredBuffer<uint4> CollectedClustersCount : register(u2);
RWStructuredBuffer<uint2> CollectedClusters : register(u3);	// x = Cluster Index; y = Instance Index

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

[RootSignature(InstanceOccludeCull_RootSig)]
[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	//*
	uint ThreadIndex = dispatchThreadId.x;// groupId.x * threadBlockSize + threadIDInGroup.x;
	if (ThreadIndex < VisibleInstanceCount[0].x)
	{
		uint InstanceIndex = VisibleInstanceIndex[ThreadIndex];
		// This tile intersect view frustum, need to cull instances one by one
		uint MeshIndex = InstanceMetaInfo[InstanceIndex].Info1.x;
		uint DrawCmdIndex = InstanceMetaInfo[InstanceIndex].Info1.y;

		// Transform primitive bbox
		float4 MinEdge = PrimitiveBBoxes[MeshIndex].MinEdge;
		float4 MaxEdge = PrimitiveBBoxes[MeshIndex].MaxEdge;
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
		float Zzz[8];
		for (int i = 0; i < 8; i++)
		{
			//transform world space aaBox to NDC
			float4 ClipPos = mul(float4(BBoxCorners[i], 1), ViewProjection);

			Zzz[i] = ClipPos.z / ClipPos.w;

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
			// Increate visible instances count
			uint CurrentInstanceCount;
			InterlockedAdd(OutputDrawCommandBuffer[DrawCmdIndex].Params.y, 1, CurrentInstanceCount);

			// Copy instance data to compaced position
			CompactInstanceData[DrawCommandBuffer[DrawCmdIndex].Param + CurrentInstanceCount] = InstanceData[InstanceIndex];

			// Modify Draw command
			OutputDrawCommandBuffer[DrawCmdIndex].Params.x = DrawCommandBuffer[DrawCmdIndex].Params.x;
			//OutputDrawCommandBuffer[DrawCmdIndex].Params.x = DrawCommandBuffer[DrawCmdIndex].Params.x;
			OutputDrawCommandBuffer[DrawCmdIndex].Params.z = DrawCommandBuffer[DrawCmdIndex].Params.z;
			OutputDrawCommandBuffer[DrawCmdIndex].Params.w = DrawCommandBuffer[DrawCmdIndex].Params.w;
			OutputDrawCommandBuffer[DrawCmdIndex].Param = DrawCommandBuffer[DrawCmdIndex].Param;

			// Collect clusters for culling
			uint ClusterBegin = InstanceMetaInfo[InstanceIndex].Info2.x;
			uint ClusterCount = InstanceMetaInfo[InstanceIndex].Info2.y;
			uint CurrentClusterIndex;
			InterlockedAdd(CollectedClustersCount[0].x, ClusterCount, CurrentClusterIndex);
			for (uint i = 0; i < ClusterCount; ++i)
			{
				CollectedClusters[CurrentClusterIndex + i].x = ClusterBegin + i;
				CollectedClusters[CurrentClusterIndex + i].y = InstanceIndex;
			}
		}
	}
	//*/
}
