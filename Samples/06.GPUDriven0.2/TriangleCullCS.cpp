/*
	TiX Engine v2.0 Copyright (C) 2018~2021
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
}

void FTriangleCullCS::UpdataComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InFrustumUniform,
	FMeshBufferPtr InSceneMeshBuffer,
	FInstanceBufferPtr InInstanceData,
	FGPUCommandBufferPtr InCommandBuffer,
	FTexturePtr InHiZTexture,
	FUniformBufferPtr InVisibleClusters,
	uint32 InTotalTrianglesInScene
)
{
	ViewFrustumUniform = InFrustumUniform;

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
	if (ViewFrustumUniform != nullptr)
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
		RHI->SetComputeConstantBuffer(0, ViewFrustumUniform);
		//RHI->SetComputeConstantBuffer(1, CollectedCountUniform);
		RHI->SetComputeResourceTable(1, ResourceTable);

		// Indirect works correct, but wrong capture in RenderDoc
		RHI->ExecuteGPUComputeCommands(TriangleCullingCB);
	}
}