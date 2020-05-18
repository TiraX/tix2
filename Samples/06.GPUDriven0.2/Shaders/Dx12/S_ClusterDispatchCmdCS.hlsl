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

#define ComputeDispatchCmd_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(UAV(u0, numDescriptors=1)),"


cbuffer FVisibleCount : register(b0)
{
	uint4 Count;
};

RWStructuredBuffer<uint4> DispatchThreadCount : register(u0);

[RootSignature(ComputeDispatchCmd_RootSig)]
[numthreads(1, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	DispatchThreadCount[0].x = uint((Count.x + 127) / 128);
	DispatchThreadCount[0].y = uint((Count.y + 127) / 128);
	DispatchThreadCount[0].z = uint((Count.z + 127) / 128);
	DispatchThreadCount[0].w = uint((Count.w + 127) / 128);
}
