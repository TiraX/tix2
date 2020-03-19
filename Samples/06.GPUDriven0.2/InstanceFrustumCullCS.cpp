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
	ResourceTable = RHI->CreateRenderResourceTable(6, EHT_SHADER_RESOURCE);
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
		ResourceTable->PutUniformBufferInTable(InPrimitiveBBoxes, 0);
		PrimitiveBBoxes = InPrimitiveBBoxes;
	}
	if (InstanceMetaInfo != InInstanceMetaInfo)
	{
		ResourceTable->PutUniformBufferInTable(InInstanceMetaInfo, 1);
		InstanceMetaInfo = InInstanceMetaInfo;
	}
	if (InstanceData != InInstanceData)
	{
		ResourceTable->PutInstanceBufferInTable(InInstanceData, 2);
		InstanceData = InInstanceData;

		// Update new CompactInstanceData
		CompactInstanceData = RHI->CreateEmptyInstanceBuffer(InInstanceData->GetInstancesCount(), TInstanceBuffer::InstanceStride);
		CompactInstanceData->SetResourceName("FrustumCulledInstanceData");
		RHI->UpdateHardwareResourceIB(CompactInstanceData, nullptr);
		ResourceTable->PutInstanceBufferInTable(CompactInstanceData, 4);
	}
	if (DrawCommandBuffer != InDrawCommandBuffer)
	{
		ResourceTable->PutUniformBufferInTable(InDrawCommandBuffer->GetCommandBuffer(), 3);
		DrawCommandBuffer = InDrawCommandBuffer;

		// Update new CulledDrawCommandBuffer
		CulledDrawCommandBuffer = RHI->CreateGPUCommandBuffer(
			InCommandSignature,
			InDrawCommandBuffer->GetEncodedCommandsCount(), 
			UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE | UB_FLAG_COMPUTE_WRITABLE
		);
		CulledDrawCommandBuffer->SetResourceName("FrustumCulledCommandBuffer");
		RHI->UpdateHardwareResourceGPUCommandBuffer(CulledDrawCommandBuffer);
		ResourceTable->PutUniformBufferInTable(CulledDrawCommandBuffer->GetCommandBuffer(), 5);

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

	RHI->CopyBufferRegion(CulledDrawCommandBuffer->GetCommandBuffer(), 0, ResetCommandBuffer, ResetCommandBuffer->GetTotalBufferSize());

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, FrustumUniform);
	RHI->SetComputeResourceTable(1, ResourceTable);
	RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
}

