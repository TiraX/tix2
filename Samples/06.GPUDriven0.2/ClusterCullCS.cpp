/*
	TiX Engine v2.0 Copyright (C) 2018~2019
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

	FrustumUniform = ti_new FCullUniform;

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

	// Create Zero reset command buffer
	ResetCounterBuffer = RHI->CreateUniformBuffer(sizeof(uint32) * 4, 1, UB_FLAG_INTERMEDIATE);
	uint8 * ZeroData = ti_new uint8[sizeof(uint32) * 4];
	memset(ZeroData, 0, sizeof(uint32) * 4);
	RHI->UpdateHardwareResourceUB(ResetCounterBuffer, ZeroData);
	ti_delete[] ZeroData;
}

void FClusterCullCS::UpdataComputeParams(
	FRHI * RHI,
	const vector2di& InRTSize,
	const vector3df& InViewDir,
	const FMatrix& InViewProjection,
	const SViewFrustum& InFrustum,
	FUniformBufferPtr InCollectedCount,
	FUniformBufferPtr InClusterBoundingData,
	FInstanceBufferPtr InInstanceData,
	FTexturePtr InHiZTexture,
	FUniformBufferPtr InCollectedClusters,
	FUniformBufferPtr InDispatchThreadCount
)
{
	CollectedCountUniform = InCollectedCount;

	FrustumUniform->UniformBufferData[0].RTSize = FUInt4(InRTSize.X, InRTSize.Y, FHiZDownSampleCS::HiZLevels, 0);
	FrustumUniform->UniformBufferData[0].ViewDir = InViewDir;
	FrustumUniform->UniformBufferData[0].ViewProjection = InViewProjection;
	for (int32 i = SViewFrustum::VF_FAR_PLANE; i < SViewFrustum::VF_PLANE_COUNT; ++i)
	{
		FrustumUniform->UniformBufferData[0].Planes[i] = FFloat4(
			InFrustum.Planes[i].Normal.X,
			InFrustum.Planes[i].Normal.Y,
			InFrustum.Planes[i].Normal.Z,
			InFrustum.Planes[i].D);
	}
	FrustumUniform->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

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
			VisibleClusters = RHI->CreateUniformBuffer(sizeof(uint32), InCollectedClusters->GetElements(), UB_FLAG_COMPUTE_WRITABLE | UB_FLAG_COMPUTE_WITH_COUNTER);
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

	if (FrustumUniform != nullptr)
	{
		// Copy dispatch thread group count
		RHI->SetResourceStateCB(DispatchCommandBuffer, RESOURCE_STATE_COPY_DEST);
		RHI->SetResourceStateUB(DispatchThreadCount, RESOURCE_STATE_COPY_SOURCE);
		RHI->ComputeCopyBuffer(DispatchCommandBuffer->GetCommandBuffer(), 0, DispatchThreadCount, 0, sizeof(uint32));
		RHI->SetResourceStateCB(DispatchCommandBuffer, RESOURCE_STATE_INDIRECT_ARGUMENT);

		// Reset visible cluster counter
		RHI->CopyBufferRegion(VisibleClusters, VisibleClusters->GetCounterOffset(), ResetCounterBuffer, sizeof(uint32));

		RHI->SetComputePipeline(ComputePipeline);
		RHI->SetComputeConstantBuffer(0, FrustumUniform->UniformBuffer);
		RHI->SetComputeConstantBuffer(1, CollectedCountUniform);
		RHI->SetComputeResourceTable(2, ResourceTable);

		//RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(3, 1, 1));
		RHI->ExecuteGPUComputeCommands(DispatchCommandBuffer);
	}
}