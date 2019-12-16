/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
//#include "GPUComputeUniforms.h"
//#include "GenerateTriangleCullIndirectCommand.h"
//#include "SceneMetaInfos.h"
//
//FGenerateTriangleCullIndirectCommand::FGenerateTriangleCullIndirectCommand()
//	: FComputeTask("S_GenerateTriangleCullIndirectCommandCS")
//{
//}
//
//FGenerateTriangleCullIndirectCommand::~FGenerateTriangleCullIndirectCommand()
//{
//}
//
//void FGenerateTriangleCullIndirectCommand::PrepareResources(FRHI * RHI)
//{
//	ResourceTable = RHI->CreateRenderResourceTable(3, EHT_SHADER_RESOURCE);
//
//	// Create counter reset
//	CounterReset = ti_new FCounterReset;
//	CounterReset->UniformBufferData[0].Zero = 0;
//	CounterReset->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
//}
//
//void FGenerateTriangleCullIndirectCommand::UpdateComputeArguments(
//	FRHI * RHI,
//	FUniformBufferPtr InVisibleClusters,
//	FGPUCommandBufferPtr InDrawCommandBuffer,
//	FGPUCommandBufferPtr InTriangleCullCommandBuffer)
//{
//	TI_ASSERT(InTriangleCullCommandBuffer != nullptr);
//	ResourceTable->PutUniformBufferInTable(InVisibleClusters, 0);
//	ResourceTable->PutUniformBufferInTable(InDrawCommandBuffer->GetCommandBuffer(), 1);
//	ResourceTable->PutUniformBufferInTable(InTriangleCullCommandBuffer->GetCommandBuffer(), 2);
//
//	VisibleClusters = InVisibleClusters;
//	TriangleCullCommandBuffer = InTriangleCullCommandBuffer;
//}
//
//void FGenerateTriangleCullIndirectCommand::Run(FRHI * RHI)
//{
//	RHI->SetResourceStateCB(TriangleCullCommandBuffer, RESOURCE_STATE_COPY_DEST);
//	RHI->CopyBufferRegion(TriangleCullCommandBuffer->GetCommandBuffer(), TriangleCullCommandBuffer->GetCommandBuffer()->GetCounterOffset(), CounterReset->UniformBuffer, sizeof(uint32));
//	RHI->SetResourceStateCB(TriangleCullCommandBuffer, RESOURCE_STATE_UNORDERED_ACCESS); 
//
//	RHI->SetComputePipeline(ComputePipeline);
//	TI_ASSERT(VisibleClusters->GetCounterOffset() != 0);
//	RHI->SetComputeConstantBuffer(0, VisibleClusters, VisibleClusters->GetCounterOffset());
//	RHI->SetComputeResourceTable(1, ResourceTable);
//
//	const uint32 BlockSize = 64;
//	const uint32 GroupCount = 10 * 1024 / BlockSize;
//
//	// Dispatch max count of thread
//	RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(GroupCount, 1, 1));
//}
