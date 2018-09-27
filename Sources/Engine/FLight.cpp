/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FLight.h"

namespace tix
{
	FLight::FLight(TNodeLight * Light)
	{
		InitFromLightNode(Light);
	}

	FLight::~FLight()
	{
		// Remove Light uniform buffer resource 
		if (DynamicLightBuffer != nullptr)
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(DeleteLightUniformBufferResource,
				FDynamicLightUniformBufferPtr, DynamicLightBuffer, DynamicLightBuffer,
				{
					TI_TODO("Manually call Destroy() is un-safe. Make a better solution to release render resources");
					DynamicLightBuffer->UniformBuffer->Destroy();
				});
			DynamicLightBuffer = nullptr;
		}
	}

	void FLight::InitFromLightNode(TNodeLight * Light)
	{
		// create uniform buffer
		if (DynamicLightBuffer == nullptr)
			DynamicLightBuffer = ti_new FDynamicLightUniformBuffer();

		DynamicLightBuffer->UniformBufferData.LightPosition = Light->GetAbsolutePosition();
		SColorf LColor(Light->GetColor());
		LColor *= Light->GetIntensity();
		DynamicLightBuffer->UniformBufferData.LightColor = LColor;
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(InitLightUniformBuffer,
			FDynamicLightUniformBufferPtr, DynamicLightBuffer, DynamicLightBuffer,
			{
				DynamicLightBuffer->InitUniformBuffer();
			});


		//if (Scene->HasSceneFlag(FScene::ViewProjectionDirty))
		//{
		//	const FViewProjectionInfo& VPInfo = Scene->GetViewProjection();
		//	ViewUniformBuffer->UniformBufferData.ViewProjection = VPInfo.MatProj * VPInfo.MatView;
		//	ViewUniformBuffer->UniformBufferData.ViewDir = VPInfo.CamDir;
		//	ViewUniformBuffer->UniformBufferData.ViewPos = VPInfo.CamPos;

		//	ViewUniformBuffer->InitUniformBuffer();

		//	// remove vp dirty flag
		//	Scene->SetSceneFlag(FScene::ViewProjectionDirty, false);
		//}
	}
}
