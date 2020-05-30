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

#define CompactDrawCommands_RootSig \
    "CBV(b0)," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=1))," 

cbuffer FCommandInfo : register(b0)
{
    uint4 Info;
};

struct FDrawInstanceCommand
{
	//uint32 IndexCountPerInstance;
	//uint32 InstanceCount; // Trick: also the StartIndexLocation in the expanded index buffer
	//uint32 StartIndexLocation;
	//uint32 BaseVertexLocation;
	//uint32 StartInstanceLocation;
    uint4 Params;
    uint Param;
};

StructuredBuffer<FDrawInstanceCommand> DrawCommandBuffer : register(t0);
AppendStructuredBuffer<FDrawInstanceCommand> CompactCommands : register(u0);

#define threadBlockSize 128

[RootSignature(CompactDrawCommands_RootSig)]
[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint DrawCmdIndex = dispatchThreadId.x;
    
    if (DrawCmdIndex < Info.x && DrawCommandBuffer[DrawCmdIndex].Params.x > 0)
    {
        CompactCommands.Append(DrawCommandBuffer[DrawCmdIndex]);
    }
}