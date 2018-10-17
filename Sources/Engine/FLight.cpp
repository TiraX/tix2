/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FLight.h"
#include "FSceneLights.h"

namespace tix
{
	FLight::FLight(TNodeLight * Light)
		: LightIndex(uint32(-1))
	{
		InitFromLightNode(Light);
	}

	FLight::~FLight()
	{
		TI_TODO("Figure out a better way to destroy render resource, call destructor in render thread ?.");
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
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(AddLightToFScene,
			FLightPtr, InLight, this,
			{
				InLight->InitRenderResource_RenderThread();
			});
	}

	void FLight::InitRenderResource_RenderThread()
	{
		TI_ASSERT(IsRenderThread());
		DynamicLightBuffer->InitUniformBuffer();
		TI_ASSERT(LightIndex == uint32(-1));
		uint32 Index = FRenderThread::Get()->GetRenderScene()->GetSceneLights()->AddLightUniformBuffer(DynamicLightBuffer->UniformBuffer);
		SetLightIndex(Index);
	}
}
