/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GPUInstanceFrustumCullCS.h"
#include "SceneMetaInfos.h"

FGPUInstanceFrustumCullCS::FGPUInstanceFrustumCullCS()
	: FComputeTask("S_InstanceFrustumCullCS")
{
}

FGPUInstanceFrustumCullCS::~FGPUInstanceFrustumCullCS()
{
}

void FGPUInstanceFrustumCullCS::PrepareResources(FRHI * RHI)
{
	// Resource table for Compute cull shader
	ResourceTable = RHI->CreateRenderResourceTable(4, EHT_SHADER_RESOURCE);
}

void FGPUInstanceFrustumCullCS::UpdateComputeArguments(
	FRHI * RHI, 
	FUniformBufferPtr PrimitiveBBoxes,
	FUniformBufferPtr InstanceMetaInfo,
	FInstanceBufferPtr SceneInstanceData,
	FUniformBufferPtr InFrustumUniform)
{
	if (VisibilityResult == nullptr || VisibilityResult->GetElements() != InstanceMetaInfo->GetElements())
	{
		// Create visibility buffer to save tile visibility result
		VisibilityResult = RHI->CreateUniformBuffer(sizeof(uint32), InstanceMetaInfo->GetElements(), UB_FLAG_COMPUTE_WRITABLE);
		RHI->UpdateHardwareResourceUB(VisibilityResult, nullptr);
	}

	TI_TODO("Does this resource table, need to re-create?");
	ResourceTable->PutUniformBufferInTable(PrimitiveBBoxes, 0);
	ResourceTable->PutUniformBufferInTable(InstanceMetaInfo, 1);
	ResourceTable->PutInstanceBufferInTable(SceneInstanceData, 2);
	ResourceTable->PutUniformBufferInTable(VisibilityResult, 3);

	FrustumUniform = InFrustumUniform;
}

void FGPUInstanceFrustumCullCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	const uint32 DispatchSize = VisibilityResult->GetElements() / BlockSize;

	if (FrustumUniform != nullptr)
	{
		RHI->SetComputePipeline(ComputePipeline);
		RHI->SetComputeBuffer(0, FrustumUniform);
		RHI->SetComputeResourceTable(1, ResourceTable);

		RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
	}
}
