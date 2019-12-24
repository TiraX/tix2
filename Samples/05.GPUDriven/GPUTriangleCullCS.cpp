/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GPUComputeUniforms.h"
#include "GPUTriangleCullCS.h"
#include "HiZDownSampleCS.h"

FGPUTriangleCullCS::FGPUTriangleCullCS()
	: FComputeTask("S_TriangleCullCS")
{
}

FGPUTriangleCullCS::~FGPUTriangleCullCS()
{
}

void FGPUTriangleCullCS::PrepareResources(FRHI * RHI, const vector2di& RTSize, FTexturePtr HiZTexture)
{
	CullUniform = ti_new FCullUniform;
	CullUniform->UniformBufferData[0].RTSize = FUInt4(RTSize.X, RTSize.Y, FHiZDownSampleCS::HiZLevels, 0);

	ResourceTable = RHI->CreateRenderResourceTable(8, EHT_SHADER_RESOURCE);
	ResourceTable->PutTextureInTable(HiZTexture, 5);

	// Create a command buffer that big enough for triangle culling
	TriangleCullResults = RHI->CreateUniformBuffer(sizeof(uint32), MAX_TRIANGLE_VISIBLE_COUNT, UB_FLAG_COMPUTE_WRITABLE | UB_FLAG_COMPUTE_WITH_COUNTER);
	TriangleCullResults->SetResourceName("TriangleCullResults");
	RHI->UpdateHardwareResourceUB(TriangleCullResults, nullptr);

	DebugGroup = RHI->CreateUniformBuffer(sizeof(FFloat4) * 5, MAX_TRIANGLE_VISIBLE_COUNT, UB_FLAG_COMPUTE_WRITABLE | UB_FLAG_COMPUTE_WITH_COUNTER);
	DebugGroup->SetResourceName("TriangleCullDebug");
	RHI->UpdateHardwareResourceUB(DebugGroup, nullptr);

	// Create counter reset
	CounterReset = ti_new FCounterReset;
	CounterReset->UniformBufferData[0].Zero = 0;
	CounterReset->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	//// Init GPU command buffer
	//TVector<E_GPU_COMMAND_TYPE> CommandStructure;
	//CommandStructure.reserve(5);
	//CommandStructure.push_back(GPU_COMMAND_CONSTANT);
	//CommandStructure.push_back(GPU_COMMAND_SHADER_RESOURCE);
	//CommandStructure.push_back(GPU_COMMAND_SHADER_RESOURCE);
	//CommandStructure.push_back(GPU_COMMAND_DISPATCH);

	//GPUCommandSignature = RHI->CreateGPUCommandSignature(ComputePipeline, CommandStructure);
	//GPUCommandSignature->SetResourceName("TriangleCullSig");
	//RHI->UpdateHardwareResourceGPUCommandSig(GPUCommandSignature);
	//
	//// Create a triangle cull gpu command buffer large enough for all dispatches
	//GPUCommandBuffer = RHI->CreateGPUCommandBuffer(GPUCommandSignature, 1024 * 2, UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE | UB_FLAG_COMPUTE_WRITABLE | UB_FLAG_COMPUTE_WITH_COUNTER | UB_FLAG_READBACK);
	//GPUCommandBuffer->SetResourceName("TriangleCullIndirectCommand");
	//RHI->UpdateHardwareResourceGPUCommandBuffer(GPUCommandBuffer);
}

void FGPUTriangleCullCS::UpdateComputeArguments(
	FRHI * RHI,
	const vector3df& ViewDir,
	const FMatrix& ViewProjection,
	const SViewFrustum& InFrustum,
	FMeshBufferPtr InSceneMergedMeshBuffer,
	FInstanceBufferPtr InSceneInstanceData,
	FUniformBufferPtr SceneMeshBufferInfo,
	FUniformBufferPtr InVisibleClusters
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

	// Input srvs
	ResourceTable->PutMeshBufferInTable(InSceneMergedMeshBuffer, 0, 1);
	ResourceTable->PutInstanceBufferInTable(InSceneInstanceData, 2);
	ResourceTable->PutUniformBufferInTable(SceneMeshBufferInfo, 3);
	ResourceTable->PutUniformBufferInTable(InVisibleClusters, 4);

	// Output uavs
	ResourceTable->PutUniformBufferInTable(TriangleCullResults, 6);
	ResourceTable->PutUniformBufferInTable(DebugGroup, 7);

	SceneInstanceData = InSceneInstanceData;
	VisibleClusters = InVisibleClusters;
}

void FGPUTriangleCullCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	const uint32 DispatchSize = VisibleClusters->GetElements();

	// Reset command buffer counter
	//RHI->SetResourceStateUB(TriangleCullResults, RESOURCE_STATE_COPY_DEST);
	RHI->ComputeCopyBuffer(
		TriangleCullResults,
		TriangleCullResults->GetCounterOffset(),
		CounterReset->UniformBuffer,
		0,
		sizeof(uint32));
	RHI->ComputeCopyBuffer(
		DebugGroup,
		DebugGroup->GetCounterOffset(),
		CounterReset->UniformBuffer,
		0,
		sizeof(uint32));

	//RHI->SetResourceStateCB(GPUCommandBuffer, RESOURCE_STATE_INDIRECT_ARGUMENT);
	//RHI->SetResourceStateUB(TriangleCullResults, RESOURCE_STATE_UNORDERED_ACCESS);
	RHI->SetResourceStateInsB(SceneInstanceData, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	RHI->SetComputePipeline(ComputePipeline);

	RHI->SetComputeConstantBuffer(0, CullUniform->UniformBuffer);
	RHI->SetComputeConstantBuffer(1, VisibleClusters, VisibleClusters->GetCounterOffset());
	RHI->SetComputeResourceTable(2, ResourceTable);

	RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
}
