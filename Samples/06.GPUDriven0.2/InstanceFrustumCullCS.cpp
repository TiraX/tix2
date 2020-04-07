/*
	TiX Engine v2.0 Copyright (C) 2018~2019
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
	VisibleInstanceCount = RHI->CreateUniformBuffer(sizeof(uint32) * 4, 1, UB_FLAG_COMPUTE_WRITABLE);
	VisibleInstanceCount->SetResourceName("VisibleInstanceCount");
	RHI->UpdateHardwareResourceUB(VisibleInstanceCount, nullptr);
	ResourceTable->PutUniformBufferInTable(VisibleInstanceCount, PARAM_VISIBLE_INSTANCE_COUNT);
}

void FInstanceFrustumCullCS::UpdataComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InFrustumUniform,
	FUniformBufferPtr InPrimitiveBBoxes,
	FUniformBufferPtr InInstanceMetaInfo,
	FInstanceBufferPtr InInstanceData,
	FGPUCommandSignaturePtr InCommandSignature,
	FGPUCommandBufferPtr InDrawCommandBuffer
)
{
	FrustumUniform = InFrustumUniform;

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

		// Update VisibleInstanceIndex Buffer, with max count = instances count
		VisibleInstanceIndex = RHI->CreateUniformBuffer(sizeof(uint32), ti_align(InInstanceData->GetInstancesCount(), 16), UB_FLAG_COMPUTE_WRITABLE);
		VisibleInstanceIndex->SetResourceName("VisibleInstanceIndex");
		RHI->UpdateHardwareResourceUB(VisibleInstanceIndex, nullptr);
		ResourceTable->PutUniformBufferInTable(VisibleInstanceIndex, PARAM_VISIBLE_INSTANCE_INDEX);
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
		CulledDrawCommandBuffer->SetResourceName("FrustumCulledCommandBuffer");
		RHI->UpdateHardwareResourceGPUCommandBuffer(CulledDrawCommandBuffer);
		ResourceTable->PutUniformBufferInTable(CulledDrawCommandBuffer->GetCommandBuffer(), PARAM_CULLED_DRAW_COMMAND_BUFFER);

		// Create Zero reset command buffer
		ResetCommandBuffer = RHI->CreateUniformBuffer(InCommandSignature->GetCommandStrideInBytes(), InDrawCommandBuffer->GetEncodedCommandsCount(), UB_FLAG_INTERMEDIATE);
		uint8 * ZeroData = ti_new uint8[InCommandSignature->GetCommandStrideInBytes() * InDrawCommandBuffer->GetEncodedCommandsCount()];
		memset(ZeroData, 0, InCommandSignature->GetCommandStrideInBytes() * InDrawCommandBuffer->GetEncodedCommandsCount());
		RHI->UpdateHardwareResourceUB(ResetCommandBuffer, ZeroData);
		ti_delete[] ZeroData;
	}
}

void FInstanceFrustumCullCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	const uint32 DispatchSize = (InstanceData->GetInstancesCount() + BlockSize - 1) / BlockSize;

	// Reset culled command buffer
	RHI->CopyBufferRegion(CulledDrawCommandBuffer->GetCommandBuffer(), 0, ResetCommandBuffer, ResetCommandBuffer->GetTotalBufferSize());
	// Reset VisibleInstanceIndex buffer
	RHI->ComputeCopyBuffer(VisibleInstanceCount, 0, ResetCommandBuffer, 0, sizeof(uint32) * 4);

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, FrustumUniform);
	RHI->SetComputeResourceTable(1, ResourceTable);
	RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
}

