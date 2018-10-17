/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	// Manage dynamic lights in FScene
	class FSceneLights
	{
	public:
		static const int32 MaxDynamicLightsInScene = MAX_HEAP_LIGHTS;
		FSceneLights();
		~FSceneLights();

		uint32 AddLightUniformBuffer(FUniformBufferPtr LightUniformBuffer);
		void RemoveLightUniformBuffer(uint32 Index);
	protected:
		void InitEmptyLightsBuffer();

	protected:
		FDynamicLightUniformBufferPtr EmptyDynamicLightBuffer;

		FRenderResourceTable LightsTable;
		uint32 Allocated;
	};
} // end namespace tix
