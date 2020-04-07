/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "InstanceOccludeCullCS.h"
#include "SceneMetaInfos.h"

FInstanceOccludeCullCS::FInstanceOccludeCullCS()
	: FComputeTask("S_InstanceOccludeCullCS")
{
}

FInstanceOccludeCullCS::~FInstanceOccludeCullCS()
{
}

void FInstanceOccludeCullCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
}

void FInstanceOccludeCullCS::UpdataComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InOcclusionInfo,
	FUniformBufferPtr InPrimitiveBBoxes,
	FUniformBufferPtr InInstanceMetaInfo,
	FInstanceBufferPtr InInstanceData,
	FGPUCommandSignaturePtr InCommandSignature,
	FGPUCommandBufferPtr InDrawCommandBuffer,
	FUniformBufferPtr InVisibleInstanceIndex,
	FUniformBufferPtr InVisibleInstanceCount,
	FTexturePtr InHiZTexture
)
{
	OcclusionInfo = InOcclusionInfo;

	if (PrimitiveBBoxes != InPrimitiveBBoxes)
	{
		ResourceTable->PutUniformBufferInTable(InPrimitiveBBoxes, PARAM_PRIMITIVE_BBOXES);
		PrimitiveBBoxes = InPrimitiveBBoxes;
	}
	if (InstanceMetaInfo != InInstanceMetaInfo)
	{
		ResourceTable->PutUniformBufferInTable(InInstanceMetaInfo, PARAM_INSTANCE_METAINFO);
		InstanceMetaInfo = InInstanceMetaInfo;
	}
	if (InstanceData != InInstanceData)
	{
		ResourceTable->PutInstanceBufferInTable(InInstanceData, PARAM_INSTANCE_DATA);
		InstanceData = InInstanceData;

		// Update new CompactInstanceData
		CompactInstanceData = RHI->CreateEmptyInstanceBuffer(InInstanceData->GetInstancesCount(), TInstanceBuffer::InstanceStride);
		CompactInstanceData->SetResourceName("FrustumCulledInstanceData");
		RHI->UpdateHardwareResourceIB(CompactInstanceData, nullptr);
		ResourceTable->PutInstanceBufferInTable(CompactInstanceData, PARAM_COMPACT_INSTANCE_DATA);
	}
	if (DrawCommandBuffer != InDrawCommandBuffer)
	{
		ResourceTable->PutUniformBufferInTable(InDrawCommandBuffer->GetCommandBuffer(), PARAM_DRAW_COMMAND_BUFFER);
		DrawCommandBuffer = InDrawCommandBuffer;

		// Update new CulledDrawCommandBuffer
		CulledDrawCommandBuffer = RHI->CreateGPUCommandBuffer(
			InCommandSignature,
			InDrawCommandBuffer->GetEncodedCommandsCount(),
			UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE | UB_FLAG_COMPUTE_WRITABLE
		);
		CulledDrawCommandBuffer->SetResourceName("OcclusionCulledCommandBuffer");
		RHI->UpdateHardwareResourceGPUCommandBuffer(CulledDrawCommandBuffer);
		ResourceTable->PutUniformBufferInTable(CulledDrawCommandBuffer->GetCommandBuffer(), PARAM_CULLED_DRAW_COMMAND_BUFFER);

		// Create Zero reset command buffer
		ResetCommandBuffer = RHI->CreateUniformBuffer(InCommandSignature->GetCommandStrideInBytes(), InDrawCommandBuffer->GetEncodedCommandsCount(), UB_FLAG_INTERMEDIATE);
		uint8 * ZeroData = ti_new uint8[InCommandSignature->GetCommandStrideInBytes() * InDrawCommandBuffer->GetEncodedCommandsCount()];
		memset(ZeroData, 0, InCommandSignature->GetCommandStrideInBytes() * InDrawCommandBuffer->GetEncodedCommandsCount());
		RHI->UpdateHardwareResourceUB(ResetCommandBuffer, ZeroData);
		ti_delete[] ZeroData;
	}
	if (VisibleInstanceIndex != InVisibleInstanceIndex)
	{
		ResourceTable->PutUniformBufferInTable(InVisibleInstanceIndex, PARAM_VISIBLE_INSTANCE_INDEX);
		VisibleInstanceIndex = InVisibleInstanceIndex;
	}
	if (VisibleInstanceCount != InVisibleInstanceCount)
	{
		ResourceTable->PutUniformBufferInTable(InVisibleInstanceCount, PARAM_VISIBLE_INSTANCE_COUNT);
		VisibleInstanceCount = InVisibleInstanceCount;
	}
	if (HiZTexture != InHiZTexture)
	{
		ResourceTable->PutTextureInTable(InHiZTexture, PARAM_HIZ_TEXTURE);
		HiZTexture = InHiZTexture;
	}
}

void FInstanceOccludeCullCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;

	if (OcclusionInfo != nullptr)
	{
		// Reset culled command buffer
		RHI->CopyBufferRegion(CulledDrawCommandBuffer->GetCommandBuffer(), 0, ResetCommandBuffer, ResetCommandBuffer->GetTotalBufferSize());
		//RHI->CopyBufferRegion(VisibilityResult, 0, FrustumCullResult, VisibilityResult->GetTotalBufferSize());
		// Reset command buffer counter
		//RHI->ComputeCopyBuffer(VisibleInstanceClusters, VisibleInstanceClusters->GetCounterOffset(), CounterReset->UniformBuffer, 0, sizeof(uint32));

		RHI->SetComputePipeline(ComputePipeline);
		RHI->SetComputeConstantBuffer(0, OcclusionInfo);
		RHI->SetComputeResourceTable(1, ResourceTable);

		TI_TODO("Dispatch indirect , since GROUP 3 is hard coded.");
		RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(3, 1, 1));
	}
}

