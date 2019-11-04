/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "HiZDownSampleCS.h"
#include "SceneMetaInfos.h"

FHiZDownSampleCS::FHiZDownSampleCS()
	: FComputeTask("S_HiZDownSampleCS")
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
		InfoUniforms[i]->UniformBufferData[0].RTInfo = FUInt4(HiZSize.X >> i, HiZSize.Y >> 1, i, 0);

		ResourceTable[i] = RHI->CreateRenderResourceTable(2, EHT_SHADER_RESOURCE);
		ResourceTable[i]->PutTextureInTable(DepthTexture, 0);
		ResourceTable[i]->PutReadWriteTextureInTable(DepthTexture, i, 1);
	}
}

void FHiZDownSampleCS::UpdateComputeArguments(FRHI * RHI, uint32 InLevel)
{
	TI_ASSERT(0);
}

void FHiZDownSampleCS::Run(FRHI * RHI)
{
	TI_ASSERT(0);
}
