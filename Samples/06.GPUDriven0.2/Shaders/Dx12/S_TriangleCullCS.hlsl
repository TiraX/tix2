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
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=6), UAV(u0, numDescriptors=2))," \
    "StaticSampler(s0, addressU = TEXTURE_ADDRESS_CLAMP, " \
                      "addressV = TEXTURE_ADDRESS_CLAMP, " \
                      "addressW = TEXTURE_ADDRESS_CLAMP, " \
                        "filter = FILTER_MIN_MAG_MIP_POINT)"

cbuffer FViewInfoUniform : register(b0)
{
	float3 ViewDir;
	float4x4 ViewProjection;
	uint4 RTSize;	// xy = size, z = max_mip_level
	float4 Planes[6];
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

StructuredBuffer<float> VertexData : register(t0);
StructuredBuffer<uint> IndexData : register(t1);
StructuredBuffer<FInstanceTransform> InstanceData : register(t2);
StructuredBuffer<FDrawInstanceCommand> DrawCommandBuffer : register(t3);
Texture2D<float> HiZTexture : register(t4);
StructuredBuffer<uint4> VisibleClusters : register(t5); // x = Cluster Begin Index; y = Indirect Draw Command Index; z = Cluster Offset in this command

RWStructuredBuffer<uint3> VisibleTriangleIndex : register(u0);
RWStructuredBuffer<FDrawInstanceCommand> OutCommands : register(u1);

SamplerState PointSampler : register(s0);

inline float3 LoadVertex(uint index, uint DataOffset)
{
	// Vertex byte stride is 24 bytes, float stride is 6
	// Position float3 = float x 3;
	// normal uint = float x 1;
	// uv0 half2 = float x 1;
	// tangent uint = float x 1;
    float3 Position;
    Position.x = VertexData[DataOffset + index * 6];
    Position.y = VertexData[DataOffset + index * 6 + 1];
    Position.z = VertexData[DataOffset + index * 6 + 2];
	//return asfloat(VertexData.Load3(index * 24));
    return Position;
}

float3 GetWorldPosition(FInstanceTransform Transform, float3 P)
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
    float level_lower = max(Mip - 1, 0);
    float2 scale = exp2(-level_lower);
    float2 a = floor(BoxUVs.xy * scale);
    float2 b = ceil(BoxUVs.zw * scale);
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
        return cull;
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
    cull = cull || (determinant(m) < 0);
    if (cull)
        return true;

	// Occlusion culling
    cull = HiZTriangle(vertices);
    if (cull)
    {
        return true;
    }

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

groupshared uint localValidTriangles;

[RootSignature(TriangleCull_RootSig)]
[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    if (threadIDInGroup.x == 0)
        localValidTriangles = 0;
    GroupMemoryBarrierWithGroupSync();

    uint ClusterIndex = groupId.x;
    uint DrawCmdIndex = VisibleClusters[ClusterIndex].y;
    uint InstanceIndex = DrawCommandBuffer[DrawCmdIndex].Param;
    uint ClusterLocalIndex = VisibleClusters[ClusterIndex].z;
    
    // Get Vertex data and Index data from draw commands
    uint VertexDataOffset = DrawCommandBuffer[DrawCmdIndex].Params.w;
    uint IndexDataOffset = DrawCommandBuffer[DrawCmdIndex].Params.z;
   
    // Calc index data offset
    uint TriangleIndex = ClusterLocalIndex * threadBlockSize + threadIDInGroup.x;
    uint IndexDataIndex = IndexDataOffset + TriangleIndex * 3;

    uint Indices[3];
    float4 Vertices[3];

    FInstanceTransform InsTrans = InstanceData[InstanceIndex];
    float3 Vertex, WorldPosition;
    uint4 oIndex = float4(0, 0, 0, 0);
	// Load vertices
	[unroll]
    for (int i = 0; i < 3; i++)
    {
        Indices[i] = IndexData[IndexDataIndex + i];
        oIndex[i] = Indices[i];
        Vertex = LoadVertex(Indices[i], VertexDataOffset);
        WorldPosition = GetWorldPosition(InsTrans, Vertex);

		// Projection space vertices
        Vertices[i] = mul(float4(WorldPosition, 1.0), ViewProjection);
    }
    oIndex.w = IndexDataIndex;
    
	// Perform cull
    if (!CullTriangle(Indices, Vertices))
    {
		// remember visible triangle index
        uint3 TIndex;
        TIndex.x = Indices[0];
        TIndex.y = Indices[1];
        TIndex.z = Indices[2];

		// remember visible triangle count
        uint localTriangleCount;
        InterlockedAdd(localValidTriangles, 1, localTriangleCount);

        VisibleTriangleIndex[IndexDataIndex / 3 + localTriangleCount] = TIndex;
    }

	// record indirect draw commands
    GroupMemoryBarrierWithGroupSync();
    if (threadIDInGroup.x == 0 && localValidTriangles > 0)
    {
        uint IndicesCount;
        InterlockedAdd(OutCommands[DrawCmdIndex].Params.x, localValidTriangles * 3, IndicesCount);
        OutCommands[DrawCmdIndex].Params.y = 1;
        OutCommands[DrawCmdIndex].Params.z = DrawCommandBuffer[DrawCmdIndex].Params.z;
        OutCommands[DrawCmdIndex].Params.w = DrawCommandBuffer[DrawCmdIndex].Params.w;
        OutCommands[DrawCmdIndex].Param = DrawCommandBuffer[DrawCmdIndex].Param;
    }
}
