/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TNodeLight.h"

namespace tix
{
	TNodeLight::TNodeLight(TNode* parent)
		: TNode(TNodeLight::NODE_TYPE, parent)
		, Intensity(1.0f)
		, LightColor(255,255,255,255)
		, LightFlag(0)
	{
	}

	TNodeLight::~TNodeLight()
	{
		if (LightResource != nullptr)
		{
			// Remove FLight from FSceneLights
			FLightPtr InLight = LightResource;
			ENQUEUE_RENDER_COMMAND(RemoveFLightFromFScene)(
				[InLight]()
				{
					InLight->RemoveFromSceneLights_RenderThread();
				});
			LightResource = nullptr;
		}
	}

	void TNodeLight::UpdateAbsoluteTransformation()
	{
		TNode::UpdateAbsoluteTransformation();
		if (NodeFlag & ENF_ABSOLUTETRANSFORMATION_UPDATED)
		{
			// Calculate affect box
			const float MinimumIntensity = 0.01f;
			float AttenuationDistance = sqrt(Intensity / MinimumIntensity);
			AffectBox.MinEdge = vector3df(-AttenuationDistance, -AttenuationDistance, -AttenuationDistance);
			AffectBox.MaxEdge = vector3df(AttenuationDistance, AttenuationDistance, AttenuationDistance);

			AffectBox.move(AbsoluteTransformation.getTranslation());

			// Update FLight Position in render thread
			FLightPtr InLight = LightResource;
			vector3df InPosition = GetAbsolutePosition();
			ENQUEUE_RENDER_COMMAND(UpdateFLightPosition)(
				[InLight, InPosition]()
				{
					InLight->UpdateLightPosition_RenderThread(InPosition);
				});

			// Notify scene , lights get dirty
			TEngine::Get()->GetScene()->SetSceneFlag(SF_LIGHTS_DIRTY, true);
		}

		TEngine::Get()->GetScene()->AddToActiveList(ESLT_LIGHTS, this);
	}

	void TNodeLight::SetRotate(const quaternion &rotate)
	{
		// keep this empty, light do not need rotate
	}

	void TNodeLight::SetScale(const vector3df& scale)
	{
		// keep this empty, light do not need scale
	}

	void TNodeLight::CreateFLight()
	{
		TI_ASSERT(LightResource == nullptr);
		LightResource = ti_new FLight(this);

		// Add FLight to FSceneLights
		FLightPtr InLight = LightResource;
		ENQUEUE_RENDER_COMMAND(AddFLightToFScene)(
			[InLight]()
			{
				InLight->AddToSceneLights_RenderThread();
			});
	}
}
