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
	// Create visibility buffer to save tile visibility result
	VisibilityResult = RHI->CreateUniformBuffer(sizeof(uint32), MAX_INSTANCES_IN_SCENE, UB_FLAG_COMPUTE_WRITABLE);
	RHI->UpdateHardwareResourceUB(VisibilityResult, nullptr);

	// Resource table for Compute cull shader
	ResourceTable = RHI->CreateRenderResourceTable(5, EHT_SHADER_RESOURCE);
}

void FGPUInstanceFrustumCullCS::UpdateComputeArguments(
	FRHI * RHI, 
	FUniformBufferPtr InTileVisbleInfo,
	FUniformBufferPtr PrimitiveBBoxes,
	FUniformBufferPtr InstanceMetaInfo,
	FInstanceBufferPtr SceneInstanceData,
	FUniformBufferPtr InFrustumUniform)
{
	TI_TODO("Does this resource table, need to re-create?");
	//ResourceTable->PutBufferInTable(SceneTileMetaInfoUniformBuffer, 0);
	ResourceTable->PutUniformBufferInTable(InTileVisbleInfo, 0);
	ResourceTable->PutUniformBufferInTable(PrimitiveBBoxes, 1);
	ResourceTable->PutUniformBufferInTable(InstanceMetaInfo, 2);
	ResourceTable->PutInstanceBufferInTable(SceneInstanceData, 3);
	ResourceTable->PutUniformBufferInTable(VisibilityResult, 4);

	FrustumUniform = InFrustumUniform;
}

void FGPUInstanceFrustumCullCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	const uint32 DispatchSize = MAX_SCENE_TILE_META_NUM / BlockSize;

	if (FrustumUniform != nullptr)
	{
		RHI->SetComputePipeline(ComputePipeline);
		RHI->SetComputeBuffer(0, FrustumUniform);
		RHI->SetComputeResourceTable(1, ResourceTable);

		RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
	}
}
