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
	{
	}

	TNodeEnvironment::~TNodeEnvironment()
	{
	}

	void TNodeEnvironment::SetMainLightDirection(const vector3df& InDir)
	{
		EnvInfo.MainLightDirection = InDir;
		EnvInfo.MainLightDirection.normalize();
		EnvFlags |= ENVF_MAIN_LIGHT_DIRTY;
	}

	void TNodeEnvironment::SetMainLightColor(const SColorf& InColor)
	{
		EnvInfo.MainLightColor = InColor;
		EnvFlags |= ENVF_MAIN_LIGHT_DIRTY;
	}

	void TNodeEnvironment::SetMainLightIntensity(float InIntensity)
	{
		EnvInfo.MainLightIntensity = InIntensity;
		EnvFlags |= ENVF_MAIN_LIGHT_DIRTY;
	}

	void TNodeEnvironment::SetSkyLightSH3(const float* SH3Data)
	{
		EnvInfo.SkyIrradiance.Init(SH3Data, FSHVectorRGB3::NumTotalFloats);
		EnvFlags |= ENVF_MAIN_LIGHT_DIRTY;
	}

	void TNodeEnvironment::UpdateAllTransformation()
	{
		TNode::UpdateAllTransformation();
		if ((EnvFlags & ENVF_MAIN_LIGHT_DIRTY) != 0)
		{
			// Notify render thread
			FEnvironmentInfo _EnvInfo = EnvInfo;
			ENQUEUE_RENDER_COMMAND(UpdateEnvInfoRenderThread)(
				[_EnvInfo]()
				{
					FScene* Scene = FRenderThread::Get()->GetRenderScene();
					Scene->SetEnvironmentInfo(_EnvInfo);
				});

			EnvFlags &= ~ENVF_MAIN_LIGHT_DIRTY;
		}
	}
}
