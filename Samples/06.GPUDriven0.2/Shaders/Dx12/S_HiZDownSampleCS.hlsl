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

#define HiZDownSample_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=1))" 

cbuffer DownSampleInfo : register(b0)
{
	// Down sampled size
	uint4 RTInfo;	// xy = Dest RT Size, z = Source RT MipLevel
};

Texture2D<float> SourceRT : register(t0);
RWTexture2D<float> DownSampledRT : register(u0);

#define threadBlockSize 16

[RootSignature(HiZDownSample_RootSig)]
[numthreads(threadBlockSize, threadBlockSize, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
	if (all(threadID.xy < RTInfo.xy))
	{
		uint Mip = RTInfo.z;
		uint ParentMip = Mip - 1;
		uint2 SourcePosition = threadID.xy * 2;
		float4 Depths;
		Depths.x = SourceRT.mips[ParentMip][SourcePosition];
		Depths.y = SourceRT.mips[ParentMip][SourcePosition + uint2(1, 0)];
		Depths.z = SourceRT.mips[ParentMip][SourcePosition + uint2(0, 1)];
		Depths.w = SourceRT.mips[ParentMip][SourcePosition + uint2(1, 1)];

		//find and return max depth
		DownSampledRT[threadID.xy] = max(max(Depths.x, Depths.y), max(Depths.z, Depths.w));
	}
}