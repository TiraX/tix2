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

void FTriangleCullCS::PrepareResources(FRHI * RHI, uint32 InMaxClustersInScene)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);

	FrustumUniform = ti_new FCullUniform;

	// Init GPU triangle cull command buffer signature
	TVector<E_GPU_COMMAND_TYPE> CommandStructure;
	CommandStructure.resize(1);
	CommandStructure[0] = GPU_COMMAND_DISPATCH;

	TriangleCullingCSig = RHI->CreateGPUCommandSignature(nullptr, CommandStructure);
	RHI->UpdateHardwareResourceGPUCommandSig(TriangleCullingCSig);

	TriangleCullingCB = RHI->CreateGPUCommandBuffer(TriangleCullingCSig, 1, UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE);
	TriangleCullingCB->SetResourceName("ClusterCullDispatchCB");
	TriangleCullingCB->EncodeSetDispatch(0, 0, 1, 1, 1);
	RHI->UpdateHardwareResourceGPUCommandBuffer(TriangleCullingCB);

	// Create Zero reset command buffer
	TI_TODO("Create a unified ResetCounter buffer.");
	ResetCounterBuffer = RHI->CreateUniformBuffer(sizeof(uint32) * 4, 1, UB_FLAG_INTERMEDIATE);
	uint8 * ZeroData = ti_new uint8[sizeof(uint32) * 4];
	memset(ZeroData, 0, sizeof(uint32) * 4);
	RHI->UpdateHardwareResourceUB(ResetCounterBuffer, ZeroData);
	ti_delete[] ZeroData;
}

void FTriangleCullCS::UpdataComputeParams(
	FRHI * RHI,
	const vector2di& InRTSize,
	const vector3df& InViewDir,
	const FMatrix& InViewProjection,
	const SViewFrustum& InFrustum,
	FInstanceBufferPtr InInstanceData,
	FGPUCommandBufferPtr InDrawCommandBuffer,
	FTexturePtr InHiZTexture
)
{
	TI_TODO("Pass in an unified Frustum Uniform, do NOT create everywhere");
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

	if (InstanceData != InInstanceData)
	{
		ResourceTable->PutInstanceBufferInTable(InInstanceData, SRV_INSTANCE_DATA);
		InstanceData = InInstanceData;
	}
	if (DrawCommandBuffer != InDrawCommandBuffer)
	{
		ResourceTable->PutUniformBufferInTable(InDrawCommandBuffer->GetCommandBuffer(), SRV_DRAW_COMMANDS);
		DrawCommandBuffer = InDrawCommandBuffer;
	}
	if (HiZTexture != InHiZTexture)
	{
		ResourceTable->PutTextureInTable(InHiZTexture, SRV_HIZ_TEXTURE);
		HiZTexture = InHiZTexture;
	}
}

void FTriangleCullCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;

	if (FrustumUniform != nullptr)
	{
		// Copy dispatch thread group count
		//RHI->SetResourceStateCB(TriangleCullingCB, RESOURCE_STATE_COPY_DEST);
		//RHI->ComputeCopyBuffer(TriangleCullingCB->GetCommandBuffer(), 0, DispatchThreadCount, 0, sizeof(uint32));
		//RHI->SetResourceStateCB(TriangleCullingCB, RESOURCE_STATE_INDIRECT_ARGUMENT);

		//// Reset visible cluster counter
		//RHI->CopyBufferRegion(VisibleClusters, VisibleClusters->GetCounterOffset(), ResetCounterBuffer, sizeof(uint32));

		//RHI->SetComputePipeline(ComputePipeline);
		//RHI->SetComputeConstantBuffer(0, FrustumUniform->UniformBuffer);
		//RHI->SetComputeConstantBuffer(1, CollectedCountUniform);
		//RHI->SetComputeResourceTable(2, ResourceTable);

		////RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(3, 1, 1));
		//RHI->ExecuteGPUComputeCommands(TriangleCullingCB);
	}
}