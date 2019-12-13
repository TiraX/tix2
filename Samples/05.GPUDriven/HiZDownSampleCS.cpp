/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "HiZDownSampleCS.h"
#include "SceneMetaInfos.h"

FHiZDownSampleCS::FHiZDownSampleCS()
	: FComputeTask("S_HiZDownSampleCS")
	, ActiveLevel(0)
{
}

FHiZDownSampleCS::~FHiZDownSampleCS()
{
}

void FHiZDownSampleCS::PrepareResources(FRHI * RHI, const vector2di& HiZSize, FTexturePtr DepthTexture)
{
	for (uint32 i = 1 ; i < HiZLevels ; ++ i)
	{
		InfoUniforms[i] = ti_new FHiZDownSampleInfo;
		InfoUniforms[i]->UniformBufferData[0].RTInfo = FUInt4(HiZSize.X >> i, HiZSize.Y >> i, i, 0);
		InfoUniforms[i]->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

		ResourceTable[i] = RHI->CreateRenderResourceTable(2, EHT_SHADER_RESOURCE);
		ResourceTable[i]->PutTextureInTable(DepthTexture, 0);
		ResourceTable[i]->PutRWTextureInTable(DepthTexture, i, 1);
	}
}

void FHiZDownSampleCS::UpdateComputeArguments(FRHI * RHI, uint32 InLevel)
{
	ActiveLevel = InLevel;
}

void FHiZDownSampleCS::Run(FRHI * RHI)
{
	const uint32 ThreadBlockSize = 16;
	vector3di DispatchSize;
	DispatchSize.X = (InfoUniforms[ActiveLevel]->UniformBufferData[0].RTInfo.X + (ThreadBlockSize - 1)) / ThreadBlockSize;
	DispatchSize.Y = (InfoUniforms[ActiveLevel]->UniformBufferData[0].RTInfo.Y + (ThreadBlockSize - 1)) / ThreadBlockSize;
	DispatchSize.Z = 1;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, InfoUniforms[ActiveLevel]->UniformBuffer);
	RHI->SetComputeResourceTable(1, ResourceTable[ActiveLevel]);

	RHI->DispatchCompute(vector3di(ThreadBlockSize, ThreadBlockSize, 1), DispatchSize);
}
