/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TNodeEnvironment.h"

namespace tix
{
	TNodeEnvironment::TNodeEnvironment(TNode* parent)
		: TNode(TNodeEnvironment::NODE_TYPE, parent)
		, MainLightDirection(0, 0, -1)
		, MainLightColor(1.f, 1.f, 1.f, 1.f)
		, MainLightIntensity(3.14f)
	{
	}

	TNodeEnvironment::~TNodeEnvironment()
	{
	}

	void TNodeEnvironment::SetMainLightDirection(const vector3df& InDir)
	{
		MainLightDirection = InDir;
	}

	void TNodeEnvironment::SetMainLightColor(const SColorf& InColor)
	{
		MainLightColor = InColor;
	}

	void TNodeEnvironment::SetMainLightIntensity(float InIntensity)
	{
		MainLightIntensity = InIntensity;
	}
}
