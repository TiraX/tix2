/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "InstanceFrustumCullCS.h"
#include "SceneMetaInfos.h"

FInstanceFrustumCullCS::FInstanceFrustumCullCS()
	: FComputeTask("S_InstanceFrustumCullCS")
{
}

FInstanceFrustumCullCS::~FInstanceFrustumCullCS()
{
}

void FInstanceFrustumCullCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	CollectedClustersCount = RHI->CreateUniformBuffer(sizeof(uint32) * 4, 1, UB_FLAG_COMPUTE_WRITABLE);
	CollectedClustersCount->SetResourceName("CollectedClustersCount");
	RHI->UpdateHardwareResourceUB(CollectedClustersCount, nullptr);
	ResourceTable->PutUniformBufferInTable(CollectedClustersCount, UAV_COLLECTED_CLUSTERS_COUNT);
}

void FInstanceFrustumCullCS::UpdataComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InFrustumUniform,
	FUniformBufferPtr InPrimitiveBBoxes,
	FUniformBufferPtr InInstanceMetaInfo,
	FInstanceBufferPtr InInstanceData,
	FGPUCommandSignaturePtr InCommandSignature,
	FGPUCommandBufferPtr InDrawCommandBuffer,
	uint32 InTotalClustersCount
)
{
	FrustumUniform = InFrustumUniform;

	if (PrimitiveBBoxes != InPrimitiveBBoxes)
	{
		ResourceTable->PutUniformBufferInTable(InPrimitiveBBoxes, SRV_PRIMITIVE_BBOXES);
		PrimitiveBBoxes = InPrimitiveBBoxes;
	}
	if (InstanceMetaInfo != InInstanceMetaInfo)
	{
		ResourceTable->PutUniformBufferInTable(InInstanceMetaInfo, SRV_INSTANCE_METAINFO);
		InstanceMetaInfo = InInstanceMetaInfo;
	}
	if (InstanceData != InInstanceData)
	{
		ResourceTable->PutInstanceBufferInTable(InInstanceData, SRV_INSTANCE_DATA);
		InstanceData = InInstanceData;
	}
	if (DrawCommandBuffer != InDrawCommandBuffer)
	{
		ResourceTable->PutUniformBufferInTable(InDrawCommandBuffer->GetCommandBuffer(), SRV_DRAW_COMMAND_BUFFER);
		DrawCommandBuffer = InDrawCommandBuffer;

		// Update new CulledDrawCommandBuffer
		CulledDrawCommandBuffer = RHI->CreateGPUCommandBuffer(
			InCommandSignature,
			InDrawCommandBuffer->GetEncodedCommandsCount(), 
			UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE | UB_FLAG_COMPUTE_WRITABLE | UB_FLAG_COMPUTE_WITH_COUNTER
		);
		CulledDrawCommandBuffer->SetResourceName("FrustumCulledCommandBuffer");
		RHI->UpdateHardwareResourceGPUCommandBuffer(CulledDrawCommandBuffer);
		ResourceTable->PutUniformBufferInTable(CulledDrawCommandBuffer->GetCommandBuffer(), UAV_CULLED_DRAW_COMMAND_BUFFER);

		// Create Zero reset command buffer
		ResetCommandBuffer = RHI->CreateUniformBuffer(sizeof(uint32) * 4, 1, UB_FLAG_INTERMEDIATE);
		uint8 * ZeroData = ti_new uint8[sizeof(uint32) * 4];
		memset(ZeroData, 0, sizeof(uint32) * 4);
		RHI->UpdateHardwareResourceUB(ResetCommandBuffer, ZeroData);
		ti_delete[] ZeroData;
	}
	if (CollectedClusters == nullptr || CollectedClusters->GetElements() != InTotalClustersCount)
	{
		TI_ASSERT(InTotalClustersCount > 0);
		CollectedClusters = RHI->CreateUniformBuffer(sizeof(uint32) * 4, InTotalClustersCount, UB_FLAG_COMPUTE_WRITABLE);
		CollectedClusters->SetResourceName("CollectedClusters");
		RHI->UpdateHardwareResourceUB(CollectedClusters, nullptr);
		ResourceTable->PutUniformBufferInTable(CollectedClusters, UAV_COLLECTED_CLUSTERS);
	}
}

void FInstanceFrustumCullCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	const uint32 DispatchSize = (DrawCommandBuffer->GetEncodedCommandsCount() + BlockSize - 1) / BlockSize;

	// Reset culled command buffer counter
	RHI->CopyBufferRegion(
		CulledDrawCommandBuffer->GetCommandBuffer(), 
		CulledDrawCommandBuffer->GetCommandBuffer()->GetCounterOffset(), 
		ResetCommandBuffer, sizeof(uint32));
	RHI->CopyBufferRegion(CollectedClustersCount, 0, ResetCommandBuffer, 4);

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, FrustumUniform);
	RHI->SetComputeResourceTable(1, ResourceTable);
	RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
}

