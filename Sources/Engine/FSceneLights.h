/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	#define MAX_SCENE_LIGHT_NUM 32

	BEGIN_UNIFORM_BUFFER_STRUCT(FSceneLightsUniformBuffer)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_ARRAY(FFloat4, LightPosition, [MAX_SCENE_LIGHT_NUM])
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_ARRAY(FFloat4, LightColor, [MAX_SCENE_LIGHT_NUM])
	END_UNIFORM_BUFFER_STRUCT(FSceneLightsUniformBuffer)

	// Manage dynamic lights in FScene
	class FSceneLights
	{
	public:
		static const int32 MaxDynamicLightsInScene = MAX_SCENE_LIGHT_NUM;
		FSceneLights();
		~FSceneLights();

		void AddLight(FLightPtr InLight);
		void RemoveLight(FLightPtr InLight);
		void UpdateLight(FLightPtr InLight);

		// InitSceneLightsUniformBufferRenderResource was send to render thread after TScene::BindLights
		void InitSceneLightsUniformBufferRenderResource();

		FSceneLightsUniformBufferPtr GetSceneLightsUniform()
		{
			return LightsUniformBuffer;
		}

	protected:
		void MarkSceneLightsDirty()
		{
			bUniformBufferDirty = true;
		}

	protected:
		FLightPtr DynamicLights[MaxDynamicLightsInScene];
		FSceneLightsUniformBufferPtr LightsUniformBuffer;
		TVector<int32> AvaibleSlot;
		bool bUniformBufferDirty;
	};
} // end namespace tix
