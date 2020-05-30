/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "CompactDrawCommandsCS.h"
#include "SceneMetaInfos.h"
#include "HiZDownSampleCS.h"

FCompactDrawCommandsCS::FCompactDrawCommandsCS()
	: FComputeTask("S_CompactDrawCommandsCS")
{
}

FCompactDrawCommandsCS::~FCompactDrawCommandsCS()
{
}

void FCompactDrawCommandsCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	// Create Zero reset command buffer
	TI_TODO("Create a unified ResetCounter buffer.");
	ResetCounterBuffer = RHI->CreateUniformBuffer(sizeof(uint32) * 4, 1, UB_FLAG_INTERMEDIATE);
	uint8 * ZeroData = ti_new uint8[sizeof(uint32) * 4];
	memset(ZeroData, 0, sizeof(uint32) * 4);
	RHI->UpdateHardwareResourceUB(ResetCounterBuffer, ZeroData);
	ti_delete[] ZeroData;
}

void FCompactDrawCommandsCS::UpdataComputeParams(
	FRHI * RHI,
	FGPUCommandBufferPtr InCommandBuffer
)
{
	if (DrawCommandBuffer != InCommandBuffer)
	{
		ResourceTable->PutUniformBufferInTable(InCommandBuffer->GetCommandBuffer(), SRV_DRAW_COMMANDS);
		DrawCommandBuffer = InCommandBuffer;

		// Create commands info uniform buffer
		CommandsInfo = RHI->CreateUniformBuffer(sizeof(uint32) * 4, 1, UB_FLAG_INTERMEDIATE);
		FUInt4 Info(DrawCommandBuffer->GetEncodedCommandsCount(), 0, 0, 0);
		RHI->UpdateHardwareResourceUB(CommandsInfo, &Info);

		// Create compact draw commands buffer resource
		if (CompactDrawCommands == nullptr || CompactDrawCommands->GetEncodedCommandsCount() != DrawCommandBuffer->GetEncodedCommandsCount())
		{
			CompactDrawCommands = RHI->CreateGPUCommandBuffer(
				DrawCommandBuffer->GetGPUCommandSignature(), 
				DrawCommandBuffer->GetEncodedCommandsCount(), 
				UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE | UB_FLAG_COMPUTE_WRITABLE | UB_FLAG_COMPUTE_WITH_COUNTER);
			CompactDrawCommands->SetResourceName("CompactDrawCommands");
			RHI->UpdateHardwareResourceGPUCommandBuffer(CompactDrawCommands);
			ResourceTable->PutUniformBufferInTable(CompactDrawCommands->GetCommandBuffer(), UAV_COMPACT_COMMANDS);
		}
	}
}

void FCompactDrawCommandsCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	if (DrawCommandBuffer != nullptr)
	{
		uint32 DispatchCount = (DrawCommandBuffer->GetEncodedCommandsCount() + BlockSize - 1 ) / BlockSize;

		// Reset output draw commands
		RHI->SetResourceStateCB(CompactDrawCommands, RESOURCE_STATE_COPY_DEST);
		RHI->CopyBufferRegion(CompactDrawCommands->GetCommandBuffer(), CompactDrawCommands->GetCommandBuffer()->GetCounterOffset(), ResetCounterBuffer, sizeof(uint32));
		RHI->SetResourceStateCB(CompactDrawCommands, RESOURCE_STATE_UNORDERED_ACCESS);
		
		RHI->SetComputePipeline(ComputePipeline);
		RHI->SetComputeConstantBuffer(0, CommandsInfo);
		RHI->SetComputeResourceTable(1, ResourceTable);

		RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchCount, 1, 1));
	}
}