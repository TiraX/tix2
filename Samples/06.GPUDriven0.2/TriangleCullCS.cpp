/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TriangleCullCS.h"
#include "SceneMetaInfos.h"
#include "HiZDownSampleCS.h"

FTriangleCullCS::FTriangleCullCS()
	: FComputeTask("S_TriangleCullCS")
{
}

FTriangleCullCS::~FTriangleCullCS()
{
}

void FTriangleCullCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	FrustumUniform = ti_new FCullUniform;

	// Init GPU triangle cull command buffer signature
	TVector<E_GPU_COMMAND_TYPE> CommandStructure;
	CommandStructure.resize(1);
	CommandStructure[0] = GPU_COMMAND_DISPATCH;

	TriangleCullingCSig = RHI->CreateGPUCommandSignature(nullptr, CommandStructure);
	TriangleCullingCSig->SetResourceName("TriangleCullingCSig");
	RHI->UpdateHardwareResourceGPUCommandSig(TriangleCullingCSig);

	TriangleCullingCB = RHI->CreateGPUCommandBuffer(TriangleCullingCSig, 1, UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE);
	TriangleCullingCB->SetResourceName("TriangleCullDispatchCB");
	TriangleCullingCB->EncodeSetDispatch(0, 0, 1, 1, 1);
	RHI->UpdateHardwareResourceGPUCommandBuffer(TriangleCullingCB);

	// Create Zero reset command buffer
	TI_TODO("Create a unified ResetCounter buffer.");
	ResetCounterBuffer = RHI->CreateUniformBuffer(sizeof(uint32) * 4, 1, UB_FLAG_INTERMEDIATE);
	uint8 * ZeroData = ti_new uint8[sizeof(uint32) * 4];
	memset(ZeroData, 0, sizeof(uint32) * 4);
	RHI->UpdateHardwareResourceUB(ResetCounterBuffer, ZeroData);
	ti_delete[] ZeroData;

	// Debug buffer
	TI_TODO("Remove debug buffer");
	DebugInfo1 = RHI->CreateUniformBuffer(sizeof(uint32) * 4, 1400 * 128, UB_FLAG_COMPUTE_WRITABLE);
	DebugInfo1->SetResourceName("DebugInfo1");
	RHI->UpdateHardwareResourceUB(DebugInfo1, nullptr);
	ResourceTable->PutUniformBufferInTable(DebugInfo1, UAV_DEBUG_INFO1);
}

void FTriangleCullCS::UpdataComputeParams(
	FRHI * RHI,
	const vector2di& InRTSize,
	const vector3df& InViewDir,
	const FMatrix& InViewProjection,
	const SViewFrustum& InFrustum,
	FMeshBufferPtr InSceneMeshBuffer,
	FInstanceBufferPtr InInstanceData,
	FGPUCommandBufferPtr InCommandBuffer,
	FTexturePtr InHiZTexture,
	FUniformBufferPtr InVisibleClusters,
	uint32 InTotalTrianglesInScene
)
{
	TI_TODO("Pass in an unified Frustum Uniform, do NOT create everywhere");
	// Create frustum uniform buffer
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

	if (MeshData != InSceneMeshBuffer)
	{
		ResourceTable->PutMeshBufferInTable(InSceneMeshBuffer, SRV_VERTEX_DATA, SRV_INDEX_DATA);
		MeshData = InSceneMeshBuffer;
	}
	if (InstanceData != InInstanceData)
	{
		ResourceTable->PutInstanceBufferInTable(InInstanceData, SRV_INSTANCE_DATA);
		InstanceData = InInstanceData;
	}
	if (DrawCommandBuffer != InCommandBuffer)
	{
		ResourceTable->PutUniformBufferInTable(InCommandBuffer->GetCommandBuffer(), SRV_DRAW_COMMANDS);
		DrawCommandBuffer = InCommandBuffer;

		// Output draw command buffer
		OutDrawCommands = RHI->CreateGPUCommandBuffer(
			DrawCommandBuffer->GetGPUCommandSignature(),
			DrawCommandBuffer->GetEncodedCommandsCount(),
			UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE | UB_FLAG_COMPUTE_WRITABLE
		);
		OutDrawCommands->SetResourceName("OutDrawCommands");
		RHI->UpdateHardwareResourceGPUCommandBuffer(OutDrawCommands);
		ResourceTable->PutUniformBufferInTable(OutDrawCommands->GetCommandBuffer(), UAV_OUTPUT_COMMANDS);

		// Create empty command buffer for reset output draw commands
		EmptyCommandBuffer = RHI->CreateGPUCommandBuffer(
			DrawCommandBuffer->GetGPUCommandSignature(),
			DrawCommandBuffer->GetEncodedCommandsCount(),
			UB_FLAG_GPU_COMMAND_BUFFER
		);
		EmptyCommandBuffer->SetResourceName("EmptyCommandBuffer");
		for (uint32 c = 0 ; c < DrawCommandBuffer->GetEncodedCommandsCount() ; ++ c)
		{
			EmptyCommandBuffer->EncodeEmptyCommand(c);
		}
		RHI->UpdateHardwareResourceGPUCommandBuffer(EmptyCommandBuffer);
	}
	if (HiZTexture != InHiZTexture)
	{
		ResourceTable->PutTextureInTable(InHiZTexture, SRV_HIZ_TEXTURE);
		HiZTexture = InHiZTexture;
	}
	if (VisibleClusters != InVisibleClusters)
	{
		ResourceTable->PutUniformBufferInTable(InVisibleClusters, SRV_VISIBLE_CLUSTERS);
		VisibleClusters = InVisibleClusters;
	}
	if (VisibleTriangleIndices == nullptr || VisibleTriangleIndices->GetElements() != InTotalTrianglesInScene)
	{
		// Create visible triangle index buffer, same size with merged mesh buffer
		VisibleTriangleIndices = RHI->CreateUniformBuffer(sizeof(uint32) * 3, InTotalTrianglesInScene, UB_FLAG_COMPUTE_WRITABLE);
		VisibleTriangleIndices->SetResourceName("VisibleTriangleIndices");
		RHI->UpdateHardwareResourceUB(VisibleTriangleIndices, nullptr);
		ResourceTable->PutUniformBufferInTable(VisibleTriangleIndices, UAV_VISIBLE_TRIANGLE_INDICES);
	}
}

void FTriangleCullCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	if (FrustumUniform != nullptr)
	{
		// Copy dispatch thread group count
		RHI->SetResourceStateCB(TriangleCullingCB, RESOURCE_STATE_COPY_DEST);
		RHI->SetResourceStateUB(VisibleClusters, RESOURCE_STATE_COPY_SOURCE);
		TI_ASSERT(VisibleClusters->GetCounterOffset() > 0);
		RHI->ComputeCopyBuffer(TriangleCullingCB->GetCommandBuffer(), 0, VisibleClusters, VisibleClusters->GetCounterOffset(), sizeof(uint32));
		RHI->SetResourceStateCB(TriangleCullingCB, RESOURCE_STATE_INDIRECT_ARGUMENT);

		// Reset output draw commands
		RHI->SetResourceStateCB(OutDrawCommands, RESOURCE_STATE_COPY_DEST);
		RHI->CopyBufferRegion(OutDrawCommands->GetCommandBuffer(), 0, EmptyCommandBuffer->GetCommandBuffer(), EmptyCommandBuffer->GetEncodedCommandsCount() * sizeof(uint32) * 5);
		RHI->SetResourceStateCB(OutDrawCommands, RESOURCE_STATE_UNORDERED_ACCESS);
		
		RHI->SetComputePipeline(ComputePipeline);
		RHI->SetComputeConstantBuffer(0, FrustumUniform->UniformBuffer);
		//RHI->SetComputeConstantBuffer(1, CollectedCountUniform);
		RHI->SetComputeResourceTable(1, ResourceTable);

		TI_TODO("Find out what wrong with indirect compute dispatch");
		RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(1351, 1, 1));
		//RHI->ExecuteGPUComputeCommands(TriangleCullingCB);
	}
}