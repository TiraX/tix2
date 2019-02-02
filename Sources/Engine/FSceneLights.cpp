/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FSceneLights.h"

namespace tix
{
	FSceneLights::FSceneLights()
		: bUniformBufferDirty(false)
	{
		// Init available slots
		AvaibleSlot.reserve(MaxDynamicLightsInScene);
		for (int32 slot = MaxDynamicLightsInScene - 1; slot >= 0; --slot)
		{
			AvaibleSlot.push_back(slot);
		}

		// Create scene lights uniform buffer
		LightsUniformBuffer = ti_new FSceneLightsUniformBuffer;
	}

	FSceneLights::~FSceneLights()
	{
		// Remove all FLightPtr refs
		for (int32 s = 0; s < MaxDynamicLightsInScene; ++s)
		{
			DynamicLights[s] = nullptr;
		}

		LightsUniformBuffer = nullptr;
	}

	void FSceneLights::AddLight(FLightPtr InLight)
	{
		uint32 LightIndex = InLight->GetLightIndex();
		if (LightIndex == uint32(-1))
		{
			// Get an available slot index
			LightIndex = AvaibleSlot.back();
			AvaibleSlot.pop_back();
			TI_ASSERT(DynamicLights[LightIndex] == nullptr);
			DynamicLights[LightIndex] = InLight;
			InLight->SetLightIndex(LightIndex);
		}
		else
		{
			TI_ASSERT(DynamicLights[LightIndex] == InLight);
		}
		LightsUniformBuffer->UniformBufferData.LightPosition[LightIndex] = InLight->GetLightPosition();
		LightsUniformBuffer->UniformBufferData.LightColor[LightIndex] = InLight->GetLightColor();
		MarkSceneLightsDirty();
	}

	void FSceneLights::RemoveLight(FLightPtr InLight)
	{
		uint32 LightIndex = InLight->GetLightIndex();
		TI_ASSERT(LightIndex < MaxDynamicLightsInScene && (DynamicLights[LightIndex] == InLight || DynamicLights[LightIndex] == nullptr));
		DynamicLights[LightIndex] = nullptr;
		AvaibleSlot.push_back(LightIndex);
	}

	void FSceneLights::UpdateLight(FLightPtr InLight)
	{
		uint32 LightIndex = InLight->GetLightIndex();
		TI_ASSERT(LightIndex != uint32(-1));
		TI_ASSERT(DynamicLights[LightIndex] == InLight);

		LightsUniformBuffer->UniformBufferData.LightPosition[LightIndex] = InLight->GetLightPosition();
		LightsUniformBuffer->UniformBufferData.LightColor[LightIndex] = InLight->GetLightColor();
		MarkSceneLightsDirty();
	}

	void FSceneLights::InitSceneLightsUniformBufferRenderResource()
	{
		if (bUniformBufferDirty)
		{
			LightsUniformBuffer->InitUniformBuffer();
			bUniformBufferDirty = false;
		}
	}
}
