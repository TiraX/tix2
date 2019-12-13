/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GPUInstanceFrustumCullCS.h"
#include "SceneMetaInfos.h"

FGPUInstanceFrustumCullCS::FGPUInstanceFrustumCullCS()
	: FComputeTask("S_InstanceFrustumCullCS")
	, InstancesNeedToCull(0)
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

inline void DoFillData(uint32 * DataDst, uint32 Count)
{
	uint32 i = 0;
	const uint32 Value = 1;
	const uint32 Count1 = Count - 8;
	for ( ; i < Count1 ; i += 8)
	{
		DataDst[i + 0] = Value;
		DataDst[i + 1] = Value;
		DataDst[i + 2] = Value;
		DataDst[i + 3] = Value;
		DataDst[i + 4] = Value;
		DataDst[i + 5] = Value;
		DataDst[i + 6] = Value;
		DataDst[i + 7] = Value;
	}
	for ( ; i < Count ; i ++)
	{
		DataDst[i] = Value;
	}
}

void FGPUInstanceFrustumCullCS::UpdateComputeArguments(
	FRHI * RHI, 
	FUniformBufferPtr PrimitiveBBoxes,
	FUniformBufferPtr InstanceMetaInfo,
	FInstanceBufferPtr SceneInstanceData,
	FUniformBufferPtr InFrustumUniform,
	uint32 InstancesCountIntersectWithFrustum)
{
	if (VisibilityResult == nullptr || VisibilityResult->GetElements() != InstanceMetaInfo->GetElements())
	{
		// Create visibility buffer to save tile visibility result
		VisibilityResult = RHI->CreateUniformBuffer(sizeof(uint32), InstanceMetaInfo->GetElements(), UB_FLAG_COMPUTE_WRITABLE);
		VisibilityResult->SetResourceName("InstanceFrustumCullResult");
		RHI->UpdateHardwareResourceUB(VisibilityResult, nullptr);

		FillVisibilityBuffer = RHI->CreateUniformBuffer(sizeof(uint32), InstanceMetaInfo->GetElements(), UB_FLAG_INTERMEDIATE);
		const uint32 ElementsCount = ti_align4(InstanceMetaInfo->GetElements());
		uint32 * FillData = ti_new uint32[ElementsCount];
		DoFillData(FillData, ElementsCount);
		RHI->UpdateHardwareResourceUB(FillVisibilityBuffer, FillData);
		ti_delete[] FillData;
	}

	TI_TODO("Does this resource table, need to re-create?");
	ResourceTable->PutUniformBufferInTable(PrimitiveBBoxes, 0);
	ResourceTable->PutUniformBufferInTable(InstanceMetaInfo, 1);
	/** SceneInstanceData use half for instance rotation. For shader global uniform do NOT support half value.
	Instance rotation use float32 for now.
	Try to use half value with these to key point:
	https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-object-byteaddressbuffer
	ByteAddressBuffer asfloat(vertexData.Load3(vertexOffset + index * 12));
	https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/f16tof32
	float f16tof32( in uint value );
	*/
	TI_TODO("Use half for instance rotation.");
	ResourceTable->PutInstanceBufferInTable(SceneInstanceData, 2);
	ResourceTable->PutUniformBufferInTable(VisibilityResult, 3);

	FrustumUniform = InFrustumUniform;
	InstancesNeedToCull = InstancesCountIntersectWithFrustum;
}

void FGPUInstanceFrustumCullCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 128;
	//const uint32 DispatchSize = (VisibilityResult->GetElements() + (BlockSize - 1)) / BlockSize;
	const uint32 DispatchSize = (InstancesNeedToCull + BlockSize - 1) / BlockSize;

	if (FrustumUniform != nullptr)
	{
		RHI->CopyBufferRegion(VisibilityResult, 0, FillVisibilityBuffer, VisibilityResult->GetTotalBufferSize());

		RHI->SetComputePipeline(ComputePipeline);
		RHI->SetComputeConstantBuffer(0, FrustumUniform);
		RHI->SetComputeResourceTable(1, ResourceTable);

		RHI->DispatchCompute(vector3di(BlockSize, 1, 1), vector3di(DispatchSize, 1, 1));
	}
}
