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

#define TriangleCull_RootSig \
	"RootConstants(num32BitConstants=1, b0), " \
	"RootConstants(num32BitConstants=1, b1), " \
	"SRV(t0) ," \
	"SRV(t1) ," \
    "DescriptorTable(CBV(b2), SRV(t2, numDescriptors=2), UAV(u0, numDescriptors=1))," \
    "StaticSampler(s0, addressU = TEXTURE_ADDRESS_CLAMP, " \
                      "addressV = TEXTURE_ADDRESS_CLAMP, " \
                      "addressW = TEXTURE_ADDRESS_CLAMP, " \
                        "filter = FILTER_MIN_MAG_MIP_POINT)"

struct ClusterMeta
{
	float4 MinEdge;
	float4 MaxEdge;
	float4 Cone;
};

struct FInstanceMetaInfo
{
	// x = primitive index
	// y = cluster index begin
	// z = cluster count
	// w = primitive was loaded.
	uint4 Info;
};

struct FInstanceTransform
{
	float4 ins_transition;
	float4 ins_transform0;
	float4 ins_transform1;
	float4 ins_transform2;
};

uint IndexOffset : register(b0);
uint InstanceIndex : register(b1);

ByteAddressBuffer VertexData : register(t0);
StructuredBuffer<uint> IndexData : register(t1);

cbuffer FViewInfoUniform : register(b2)
{
	float3 ViewDir;
	float4x4 ViewProjection;
	uint4 RTSize;	// xy = size, z = max_mip_level
	float4 Planes[6];
};
StructuredBuffer<FInstanceTransform> InstanceData : register(t2);
Texture2D<float> HiZTexture : register(t3);

AppendStructuredBuffer<uint> TriangleCullingCommand : register(u0);	// Visible triangles

SamplerState PointSampler : register(s0);

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

inline float3 LoadVertex(uint index)
{
	// Vertex stride is 24 bytes
	return asfloat(VertexData.Load3(index * 24));
}

float3 GetWorldPosition(in FInstanceTransform Transform, in float3 P)
{
	float3x3 RotMat = float3x3(Transform.ins_transform0.xyz, Transform.ins_transform1.xyz, Transform.ins_transform2.xyz);
	float3 Position = mul(P, RotMat);
	Position += Transform.ins_transition.xyz;
	return Position;
}

bool HiZTriangle(float4 vertices[3])
{
	bool cull = false;

	float minZ = 1;
	float2 minXY = 1;
	float2 maxXY = 0;

	[unroll]
	for (int i = 0; i < 3; i++)
	{
		//transform world space aaBox to NDC
		float4 ClipPos = vertices[i];

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

	if (minZ > maxDepth)
	{
		// Cull it
		cull = true;
	}
	return cull;
}

bool CullTriangle(uint indices[3], float4 vertices[3])
{
	bool cull = false;

	// Zero face cull
	if (indices[0] == indices[1]
		|| indices[1] == indices[2]
		|| indices[0] == indices[2])
	{
		cull = true;
	}

	// Backface cull.
	// Culling in homogenous coordinates
	// Read: "Triangle Scan Conversion using 2D Homogeneous Coordinates"
	//       by Marc Olano, Trey Greer
	//       http://www.cs.unc.edu/~olano/papers/2dh-tri/2dh-tri.pdf
	float3x3 m =
	{
		vertices[0].xyw, vertices[1].xyw, vertices[2].xyw
	};
	cull = cull || (determinant(m) > 0);

	// Occlusion culling
	cull = cull || HiZTriangle(vertices);

	// Frustum cull.
	int verticesInFrontOfNearPlane = 0;
	// Transform vertices[i].xy into normalized 0..1 screen space
	for (uint i = 0; i < 3; ++i)
	{
		vertices[i].xy /= vertices[i].w;
		vertices[i].xy /= 2;
		vertices[i].xy += float2(0.5, 0.5);
		if (vertices[i].w < 0)
		{
			++verticesInFrontOfNearPlane;
		}
	}

	if (verticesInFrontOfNearPlane == 3)
	{
		cull = true;
	}

	if (verticesInFrontOfNearPlane == 0)
	{
		float minx = min(min(vertices[0].x, vertices[1].x), vertices[2].x);
		float miny = min(min(vertices[0].y, vertices[1].y), vertices[2].y);
		float maxx = max(max(vertices[0].x, vertices[1].x), vertices[2].x);
		float maxy = max(max(vertices[0].y, vertices[1].y), vertices[2].y);

		cull = cull || (maxx < 0) || (maxy < 0) || (minx > 1) || (miny > 1);
	}

	return cull;
}

#define threadBlockSize 128

[RootSignature(TriangleCull_RootSig)]
[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint TriangleIndex = dispatchThreadId.x;
	uint Indices[3];
	float4 Vertices[3];

	// Load vertices
	FInstanceTransform InsTrans = InstanceData[InstanceIndex];
	[unroll]
	for (int i = 0; i < 3; i++)
	{
		Indices[i] = IndexData[IndexOffset + i];
		float3 Vertex = LoadVertex(Indices[i]);
		float3 WorldPosition = GetWorldPosition(InsTrans, Vertex);

		// Projection space vertices
		Vertices[i] = mul(float4(WorldPosition, 1.0), ViewProjection);
	}

	// Perform cull
	if (!CullTriangle(Indices, Vertices))
	{
		TriangleCullingCommand.Append(1);
	}
}
