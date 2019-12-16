/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GPUComputeUniforms.h"
#include "GPUClusterCullCS.h"
#include "HiZDownSampleCS.h"

FGPUClusterCullCS::FGPUClusterCullCS()
	: FComputeTask("S_ClusterCullCS")
{
}

FGPUClusterCullCS::~FGPUClusterCullCS()
{
}

void FGPUClusterCullCS::PrepareResources(FRHI * RHI, const vector2di& RTSize, FTexturePtr HiZTexture, FUniformBufferPtr VisibleInstanceClusters)
{
	CullUniform = ti_new FCullUniform;
	CullUniform->UniformBufferData[0].RTSize = FUInt4(RTSize.X, RTSize.Y, FHiZDownSampleCS::HiZLevels, 0);

	ResourceTable = RHI->CreateRenderResourceTable(5, EHT_SHADER_RESOURCE);
	ResourceTable->PutUniformBufferInTable(VisibleInstanceClusters, 2);
	ResourceTable->PutTextureInTable(HiZTexture, 3);

	// Create a command buffer that big enough for triangle culling
	VisibleClusters = RHI->CreateUniformBuffer(sizeof(uint32) * 4, MAX_VISIBLE_CLUSTERS_COUNT, UB_FLAG_COMPUTE_WRITABLE | UB_FLAG_COMPUTE_WITH_COUNTER);
	VisibleClusters->SetResourceName("VisibleClusters");
	RHI->UpdateHardwareResourceUB(VisibleClusters, nullptr);

	FUInt4 * ClearData = ti_new FUInt4[MAX_VISIBLE_CLUSTERS_COUNT];
	memset(ClearData, 0, sizeof(FUInt4) * MAX_VISIBLE_CLUSTERS_COUNT);
	ClusterClear = RHI->CreateUniformBuffer(sizeof(uint32) * 4, MAX_VISIBLE_CLUSTERS_COUNT);
	ClusterClear->SetResourceName("ClusterClear");
	RHI->UpdateHardwareResourceUB(ClusterClear, ClearData);
	ti_delete[] ClearData;

	// Create counter reset
	CounterReset = ti_new FCounterReset;
	CounterReset->UniformBufferData[0].Zero = 0;
	CounterReset->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	// Init GPU command buffer
	//TVector<E_GPU_COMMAND_TYPE> CommandStructure;
	//CommandStructure.reserve(1);
	//CommandStructure.push_back(GPU_COMMAND_DISPATCH);

	//GPUCommandSignature = RHI->CreateGPUCommandSignature(ComputePipeline, CommandStructure);
	//GPUCommandSignature->SetResourceName("ClusterCullSig");
	//RHI->UpdateHardwareResourceGPUCommandSig(GPUCommandSignature);
	//
	//GPUCommandBuffer = RHI->CreateGPUCommandBuffer(GPUCommandSignature, 1, UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE | UB_FLAG_COMPUTE_WRITABLE | UB_FLAG_COMPUTE_WITH_COUNTER);
	//GPUCommandBuffer->SetResourceName("ClusterCullIndirectCommand");
	//RHI->UpdateHardwareResourceGPUCommandBuffer(GPUCommandBuffer);
}

void FGPUClusterCullCS::UpdateComputeArguments(
	FRHI * RHI,
	const vector3df& ViewDir,
	const FMatrix& ViewProjection,
	const SViewFrustum& InFrustum,
	FUniformBufferPtr ClusterMetaData,
	FInstanceBufferPtr SceneInstanceData,
	FUniformBufferPtr InVisibleInstanceClusters
)
{
	CullUniform->UniformBufferData[0].ViewDir = ViewDir;
	CullUniform->UniformBufferData[0].ViewProjection = ViewProjection;
	for (int32 i = SViewFrustum::VF_FAR_PLANE; i < SViewFrustum::VF_PLANE_COUNT; ++i)
	{
		CullUniform->UniformBufferData[0].Planes[i] = FFloat4(
			InFrustum.Planes[i].Normal.X,
			InFrustum.Planes[i].Normal.Y,
			InFrustum.Planes[i].Normal.Z,
			InFrustum.Planes[i].D);
	}
	CullUniform->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	ResourceTable->PutUniformBufferInTable(ClusterMetaData, 0);
	ResourceTable->PutInstanceBufferInTable(SceneInstanceData, 1);
	// t2 is VisibleClusters, alrealy set in PrepareResource()
	// t3 is HiZTexture, already set in PrepareResource()
	// u0 is VisibleClusters, already set in PrepareResource()
	ResourceTable->PutUniformBufferInTable(VisibleClusters, 4);

	VisibleInstanceClusters = InVisibleInstanceClusters;
}

void FGPUClusterCullCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	const uint32 DispatchSize = (VisibleInstanceClusters->GetElements() + 127) / 128;
	const int32 CurrFrame = RHI->GetCurrentEncodingFrameIndex();

	// Reset command buffer counter
	//RHI->SetResourceStateUB(VisibleClusters, RESOURCE_STATE_COPY_DEST);
	RHI->ComputeCopyBuffer(
		VisibleClusters,
		0,
		ClusterClear,
		0,
		ClusterClear->GetTotalBufferSize());
	RHI->ComputeCopyBuffer(
		VisibleClusters,
		VisibleClusters->GetCounterOffset(),
		CounterReset->UniformBuffer,
		0,
		sizeof(uint32));

	//RHI->SetResourceStateCB(GPUCommandBuffer, RESOURCE_STATE_INDIRECT_ARGUMENT);
	//RHI->SetResourceStateUB(VisibleClusters, RESOURCE_STATE_UNORDERED_ACCESS);

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, CullUniform->UniformBuffer);
	RHI->SetComputeConstantBuffer(1, VisibleInstanceClusters, VisibleInstanceClusters->GetCounterOffset());
	RHI->SetComputeResourceTable(2, ResourceTable);

	//RHI->ExecuteGPUComputeCommands(GPUCommandBuffer);
	RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
}
