/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TNodeEnvironment.h"

namespace tix
{
	TNodeEnvironment::TNodeEnvironment(TNode* parent)
		: TNode(TNodeEnvironment::NODE_TYPE, parent)
		, EnvFlags(ENVF_MAIN_LIGHT_DIRTY)
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
		MainLightDirection.normalize();
		EnvFlags |= ENVF_MAIN_LIGHT_DIRTY;
	}

	void TNodeEnvironment::SetMainLightColor(const SColorf& InColor)
	{
		MainLightColor = InColor;
		EnvFlags |= ENVF_MAIN_LIGHT_DIRTY;
	}

	void TNodeEnvironment::SetMainLightIntensity(float InIntensity)
	{
		MainLightIntensity = InIntensity;
		EnvFlags |= ENVF_MAIN_LIGHT_DIRTY;
	}

	void TNodeEnvironment::UpdateAllTransformation()
	{
		TNode::UpdateAllTransformation();
		if ((EnvFlags & ENVF_MAIN_LIGHT_DIRTY) != 0)
		{
			// Notify render thread
			FEnvironmentInfo Info;
			Info.MainLightDir = MainLightDirection;
			Info.MainLightColor = MainLightColor;
			Info.MainLightIntensity = MainLightIntensity;

			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(UpdateEnvInfoRenderThread,
				FEnvironmentInfo, EnvInfo, Info,
				{
					FScene * Scene = FRenderThread::Get()->GetRenderScene();
					Scene->SetEnvironmentInfo(EnvInfo);
				});

			EnvFlags &= ~ENVF_MAIN_LIGHT_DIRTY;
		}
	}
}
