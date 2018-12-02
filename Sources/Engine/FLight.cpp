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
	}

	void FLight::InitFromLightNode(TNodeLight * Light)
	{
		Position = Light->GetAbsolutePosition();
		SColorf LColor(Light->GetColor());
		LColor *= Light->GetIntensity();
		Color = LColor;

	}
	
	void FLight::AddToSceneLights_RenderThread()
	{
		TI_ASSERT(LightIndex == uint32(-1));
		FRenderThread::Get()->GetRenderScene()->GetSceneLights()->AddLight(this);
	}

	void FLight::RemoveFromSceneLights_RenderThread()
	{
		TI_ASSERT(LightIndex != uint32(-1));
		FRenderThread::Get()->GetRenderScene()->GetSceneLights()->RemoveLight(this);
	}

	void FLight::UpdateLightPosition_RenderThread(const vector3df& InPosition)
	{
		Position = InPosition;
		FRenderThread::Get()->GetRenderScene()->GetSceneLights()->UpdateLight(this);
	}
}
