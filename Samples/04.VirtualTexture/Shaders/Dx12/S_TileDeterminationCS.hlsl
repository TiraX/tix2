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

//#define TileDetermination_RootSig \
//	"CBV(b0) ," \
//    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0)),"

#define TileDetermination_RootSig \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0)),"

//cbuffer RootConstants : register(b0)
//{
//	float4 Info;	// x, y groups
//};

Texture2D<float4> ScreenUV : register(t0);
RWStructuredBuffer<float> OutputUV : register(u0);	// UAV: Processed indirect commands

#define threadBlockSize 32

static const float vt_mips[7] = { 64.f, 32.f, 16.f, 8.f, 4.f, 2.f, 1.f };
static const int vt_mips_offset[7] = { 0, 4096, 5120, 5376, 5440, 5456, 5460 };

[RootSignature(TileDetermination_RootSig)]
[numthreads(threadBlockSize, threadBlockSize, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	float4 result = ScreenUV[dispatchThreadId.xy];
	uint mip_level = uint(result.z);
	uint vt_mip_size = vt_mips[mip_level];
	result.xy = min(float2(0.999f, 0.999f), result.xy);
	uint page_x = uint(result.x * vt_mip_size);
	uint page_y = uint(result.y * vt_mip_size);
	uint output_index = page_y * vt_mip_size + page_x + vt_mips_offset[mip_level];

	OutputUV[output_index] = 1;
}
