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

	// Init GPU dispatch command buffer signature
	TVector<E_GPU_COMMAND_TYPE> CommandStructure;
	CommandStructure.resize(1);
	CommandStructure[0] = GPU_COMMAND_DISPATCH;

	DispatchCommandSig = RHI->CreateGPUCommandSignature(nullptr, CommandStructure);
	RHI->UpdateHardwareResourceGPUCommandSig(DispatchCommandSig);

	DispatchCommandBuffer = RHI->CreateGPUCommandBuffer(DispatchCommandSig, 1, UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE);
	DispatchCommandBuffer->SetResourceName("InstanceOcclusionCullDispatchCB");
	DispatchCommandBuffer->EncodeSetDispatch(0, 0, 1, 1, 1);
	RHI->UpdateHardwareResourceGPUCommandBuffer(DispatchCommandBuffer);
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
	FTexturePtr InHiZTexture,
	FUniformBufferPtr InDispatchThreadCount
)
{
	OcclusionInfo = InOcclusionInfo;

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

		// Update new CompactInstanceData
		CompactInstanceData = RHI->CreateEmptyInstanceBuffer(InInstanceData->GetInstancesCount(), TInstanceBuffer::InstanceStride);
		CompactInstanceData->SetResourceName("OcclusionCulledInstanceData");
		RHI->UpdateHardwareResourceIB(CompactInstanceData, nullptr);
		ResourceTable->PutInstanceBufferInTable(CompactInstanceData, UAV_COMPACT_INSTANCE_DATA);
	}
	if (DrawCommandBuffer != InDrawCommandBuffer)
	{
		ResourceTable->PutUniformBufferInTable(InDrawCommandBuffer->GetCommandBuffer(), SRV_DRAW_COMMAND_BUFFER);
		DrawCommandBuffer = InDrawCommandBuffer;

		// Update new CulledDrawCommandBuffer
		CulledDrawCommandBuffer = RHI->CreateGPUCommandBuffer(
			InCommandSignature,
			InDrawCommandBuffer->GetEncodedCommandsCount(),
			UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE | UB_FLAG_COMPUTE_WRITABLE
		);
		CulledDrawCommandBuffer->SetResourceName("OcclusionCulledCommandBuffer");
		RHI->UpdateHardwareResourceGPUCommandBuffer(CulledDrawCommandBuffer);
		ResourceTable->PutUniformBufferInTable(CulledDrawCommandBuffer->GetCommandBuffer(), UAV_CULLED_DRAW_COMMAND_BUFFER);

		// Create Zero reset command buffer
		ResetCommandBuffer = RHI->CreateUniformBuffer(InCommandSignature->GetCommandStrideInBytes(), InDrawCommandBuffer->GetEncodedCommandsCount(), UB_FLAG_INTERMEDIATE);
		uint8 * ZeroData = ti_new uint8[InCommandSignature->GetCommandStrideInBytes() * InDrawCommandBuffer->GetEncodedCommandsCount()];
		memset(ZeroData, 0, InCommandSignature->GetCommandStrideInBytes() * InDrawCommandBuffer->GetEncodedCommandsCount());
		RHI->UpdateHardwareResourceUB(ResetCommandBuffer, ZeroData);
		ti_delete[] ZeroData;
	}
	if (VisibleInstanceIndex != InVisibleInstanceIndex)
	{
		ResourceTable->PutUniformBufferInTable(InVisibleInstanceIndex, SRV_VISIBLE_INSTANCE_INDEX);
		VisibleInstanceIndex = InVisibleInstanceIndex;
	}
	if (VisibleInstanceCount != InVisibleInstanceCount)
	{
		ResourceTable->PutUniformBufferInTable(InVisibleInstanceCount, SRV_VISIBLE_INSTANCE_COUNT);
		VisibleInstanceCount = InVisibleInstanceCount;
	}
	if (HiZTexture != InHiZTexture)
	{
		ResourceTable->PutTextureInTable(InHiZTexture, SRV_HIZ_TEXTURE);
		HiZTexture = InHiZTexture;
	}
	if (DispatchThreadCount != InDispatchThreadCount)
	{
		DispatchThreadCount = InDispatchThreadCount;
	}
}

void FInstanceOccludeCullCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;

	if (OcclusionInfo != nullptr)
	{
		// Reset culled command buffer
		RHI->CopyBufferRegion(CulledDrawCommandBuffer->GetCommandBuffer(), 0, ResetCommandBuffer, ResetCommandBuffer->GetTotalBufferSize());

		// Copy dispatch thread group count
		RHI->SetResourceStateCB(DispatchCommandBuffer, RESOURCE_STATE_COPY_DEST);
		RHI->SetResourceStateUB(DispatchThreadCount, RESOURCE_STATE_COPY_SOURCE);
		RHI->ComputeCopyBuffer(DispatchCommandBuffer->GetCommandBuffer(), 0, DispatchThreadCount, 0, sizeof(uint32));
		RHI->SetResourceStateCB(DispatchCommandBuffer, RESOURCE_STATE_INDIRECT_ARGUMENT);
		
		RHI->SetComputePipeline(ComputePipeline);
		RHI->SetComputeConstantBuffer(0, OcclusionInfo);
		RHI->SetComputeResourceTable(1, ResourceTable);

		//RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(3, 1, 1));
		RHI->ExecuteGPUComputeCommands(DispatchCommandBuffer);
	}
}