/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TNodeLight.h"

namespace tix
{
	TNodeLight::TNodeLight(TNode* parent)
		: TNode(TNodeLight::NODE_TYPE, parent)
		, Intensity(1.0f)
		, Diffuse(1.0f, 1.0f, 1.0f, 1.0f)
		, LightFlag(0)
	{
	}

	TNodeLight::~TNodeLight()
	{
	}

	void TNodeLight::UpdateAbsoluteTransformation()
	{
		TNode::UpdateAbsoluteTransformation();
		if (NodeFlag & ENF_ABSOLUTETRANSFORMATION_UPDATED)
		{
			// calculate affect box
			const float MinimumIntensity = 0.01f;
			float AttenuationDistance = sqrt(Intensity / MinimumIntensity);
			AffectBox.MinEdge = vector3df(-AttenuationDistance, -AttenuationDistance, -AttenuationDistance);
			AffectBox.MaxEdge = vector3df(AttenuationDistance, AttenuationDistance, AttenuationDistance);

			AbsoluteTransformation.transformBox(AffectBox);
		}

		NodeFlag &= ~ENF_ABSOLUTETRANSFORMATION_UPDATED;
		TI_TODO("Add active light to scene light list");
		//TiEngine::Get()->GetRenderStage()->AddToList(ESLT_LIGHTS, this);	// add to light list for light calculation
	}

	void TNodeLight::SetRotate(const quaternion &rotate)
	{
	}

	void TNodeLight::SetScale(const vector3df& scale)
	{
	}
}
