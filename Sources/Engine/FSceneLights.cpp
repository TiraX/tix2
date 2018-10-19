/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FSceneLights.h"

namespace tix
{
	FSceneLights::FSceneLights()
		: Allocated(0)
	{
		InitEmptyLightsBuffer();
	}

	FSceneLights::~FSceneLights()
	{
		EmptyDynamicLightBuffer = nullptr;
		LightsTable = nullptr;
	}

	void FSceneLights::InitEmptyLightsBuffer()
	{
		// Create Render resource table for all dynamic lights
		LightsTable = FRHI::Get()->GetRenderResourceHeap(EHT_UNIFORMBUFFER_LIGHT).AllocateTable(MaxDynamicLightsInScene);

		// Init default empty light buffer
		EmptyDynamicLightBuffer = ti_new FDynamicLightUniformBuffer();
		EmptyDynamicLightBuffer->InitUniformBuffer();

		// Fill empty light buffer to table
		for (int32 i = 0 ; i < MaxDynamicLightsInScene; ++ i)
		{
			LightsTable->PutUniformBufferInTable(EmptyDynamicLightBuffer->UniformBuffer, i);
		}
	}

	uint32 FSceneLights::AddLightUniformBuffer(FUniformBufferPtr LightUniformBuffer)
	{
		uint32 CurrentSlot = Allocated;
		LightsTable->PutUniformBufferInTable(LightUniformBuffer, CurrentSlot);
		++Allocated;
		TI_ASSERT(Allocated <= MaxDynamicLightsInScene);
		return CurrentSlot;
	}

	void FSceneLights::RemoveLightUniformBuffer(uint32 Index)
	{
		TI_ASSERT(Index < Allocated);
		LightsTable->PutUniformBufferInTable(EmptyDynamicLightBuffer->UniformBuffer, Index);
	}
}
