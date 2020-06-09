/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ClusterCullCS.h"
#include "SceneMetaInfos.h"
#include "HiZDownSampleCS.h"

FClusterCullCS::FClusterCullCS()
	: FComputeTask("S_ClusterCullCS")
{
}

FClusterCullCS::~FClusterCullCS()
{
}

void FClusterCullCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	// Init GPU dispatch command buffer signature
	TVector<E_GPU_COMMAND_TYPE> CommandStructure;
	CommandStructure.resize(1);
	CommandStructure[0] = GPU_COMMAND_DISPATCH;

	DispatchCommandSig = RHI->CreateGPUCommandSignature(nullptr, CommandStructure);
	RHI->UpdateHardwareResourceGPUCommandSig(DispatchCommandSig);

	DispatchCommandBuffer = RHI->CreateGPUCommandBuffer(DispatchCommandSig, 1, UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE);
	DispatchCommandBuffer->SetResourceName("ClusterCullDispatchCB");
	DispatchCommandBuffer->EncodeSetDispatch(0, 0, 1, 1, 1);
	RHI->UpdateHardwareResourceGPUCommandBuffer(DispatchCommandBuffer);
}

void FClusterCullCS::UpdataComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InViewFrustumUniform,
	FUniformBufferPtr InCollectedCount,
	FUniformBufferPtr InClusterBoundingData,
	FInstanceBufferPtr InInstanceData,
	FGPUCommandBufferPtr InDrawCommandBuffer,
	FTexturePtr InHiZTexture,
	FUniformBufferPtr InCollectedClusters,
	FUniformBufferPtr InDispatchThreadCount,
	FUniformBufferPtr InCounterResetBuffer
)
{
	CollectedCountUniform = InCollectedCount;
	ViewFrustumUniform = InViewFrustumUniform;
	CounterResetBuffer = InCounterResetBuffer;

	if (ClusterBoundingData != InClusterBoundingData)
	{
		ResourceTable->PutUniformBufferInTable(InClusterBoundingData, SRV_CLUSTER_BOUNDING_DATA);
		ClusterBoundingData = InClusterBoundingData;
	}
	if (InstanceData != InInstanceData)
	{
		ResourceTable->PutInstanceBufferInTable(InInstanceData, SRV_INSTANCE_DATA);
		InstanceData = InInstanceData;
	}
	if (DrawCommandBuffer != InDrawCommandBuffer)
	{
		ResourceTable->PutUniformBufferInTable(InDrawCommandBuffer->GetCommandBuffer(), SRV_DRAW_COMMANDS);
		DrawCommandBuffer = InDrawCommandBuffer;
	}
	if (HiZTexture != InHiZTexture)
	{
		ResourceTable->PutTextureInTable(InHiZTexture, SRV_HIZ_TEXTURE);
		HiZTexture = InHiZTexture;
	}
	if (CollectedClusters != InCollectedClusters)
	{
		ResourceTable->PutUniformBufferInTable(InCollectedClusters, SRV_COLLECTED_CLUSTERS);
		CollectedClusters = InCollectedClusters;

		if (VisibleClusters == nullptr || (VisibleClusters->GetElements() != InCollectedClusters->GetElements()))
		{
			VisibleClusters = RHI->CreateUniformBuffer(sizeof(uint32) * 4, InCollectedClusters->GetElements(), UB_FLAG_COMPUTE_WRITABLE | UB_FLAG_COMPUTE_WITH_COUNTER);
			VisibleClusters->SetResourceName("VisibleClusters");
			RHI->UpdateHardwareResourceUB(VisibleClusters, nullptr);
			ResourceTable->PutUniformBufferInTable(VisibleClusters, UAV_VISIBLE_CLUSTERS);
		}
	}
	if (DispatchThreadCount != InDispatchThreadCount)
	{
		DispatchThreadCount = InDispatchThreadCount;
	}
}

void FClusterCullCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;

	if (ViewFrustumUniform != nullptr)
	{
		// Copy dispatch thread group count
		RHI->SetResourceStateCB(DispatchCommandBuffer, RESOURCE_STATE_COPY_DEST);
		RHI->SetResourceStateUB(DispatchThreadCount, RESOURCE_STATE_COPY_SOURCE);
		RHI->ComputeCopyBuffer(DispatchCommandBuffer->GetCommandBuffer(), 0, DispatchThreadCount, 0, sizeof(uint32));
		RHI->SetResourceStateCB(DispatchCommandBuffer, RESOURCE_STATE_INDIRECT_ARGUMENT);

		// Reset visible cluster counter
		RHI->SetResourceStateUB(VisibleClusters, RESOURCE_STATE_COPY_DEST);
		RHI->CopyBufferRegion(VisibleClusters, VisibleClusters->GetCounterOffset(), CounterResetBuffer, sizeof(uint32));

		RHI->SetComputePipeline(ComputePipeline);
		RHI->SetComputeConstantBuffer(0, ViewFrustumUniform);
		RHI->SetComputeConstantBuffer(1, CollectedCountUniform);
		RHI->SetComputeResourceTable(2, ResourceTable);

		//RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(23, 1, 1));
		RHI->ExecuteGPUComputeCommands(DispatchCommandBuffer);
	}
}