/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SSSSTicker.h"

TSSSSTicker::TSSSSTicker()
{
}

TSSSSTicker::~TSSSSTicker()
{
}

void TSSSSTicker::Tick(float Dt)
{
	TScene * Scene = TEngine::Get()->GetScene();
	Scene->UpdateAllNodesTransforms();
}

void TSSSSTicker::SetupScene()
{
	TScene * Scene = TEngine::Get()->GetScene();
	// Head
	{
		const TString TargetMeshRes = "SM_Head.tres";
		TResourcePtr MeshRes = TResourceLibrary::Get()->LoadResource(TargetMeshRes);
		TMeshBufferPtr MeshBuffer = static_cast<TMeshBuffer*>(MeshRes.get());

		Scene->AddStaticMesh(MeshBuffer, MeshBuffer->GetDefaultMaterial(), false, false);
	}

	// Sky Dome
	{
		const TString TargetMeshRes = "SM_SkyDome.tres";
		TResourcePtr MeshRes = TResourceLibrary::Get()->LoadResource(TargetMeshRes);
		TMeshBufferPtr MeshBuffer = static_cast<TMeshBuffer*>(MeshRes.get());

		Scene->AddStaticMesh(MeshBuffer, MeshBuffer->GetDefaultMaterial(), false, false);
	}

	// Add lights to TScene
	static const int32 N_LIGHTS = 3;
	static const vector3df LightPosition[N_LIGHTS] = {
		vector3df(1.872f, 0.39552f, 0.018f),
		vector3df(1.91f, 1.862472f, -1.838f),
		vector3df(-0.76645f, 0.518553f, 1.773f)
	};
	static const SColor LightColor[N_LIGHTS] = {
		SColor(255, 255, 255, 255),
		SColor(255, 255, 255, 255),
		SColor(255, 255, 255, 255)
	};
	static const float LightIntensity[N_LIGHTS] = {
		1.05f * 2, 0.55f * 2, 1.55f
	};
	for (int32 l = 0; l < N_LIGHTS; ++l)
	{
		Scene->AddLight(LightPosition[l], LightIntensity[l], LightColor[l]);
	}

	// Setup Camera
	TNodeCamera* Camera = Scene->GetActiveCamera();
	Camera->SetPosition(vector3df(-0.72f, -1.14f, 0.47f) * 2.f);
	//Camera->SetPosition(vector3df(2.642f, 0.0277f, -1.623f));
	Camera->SetTarget(vector3df(0.f, 0.f, 0.45f));
	//Camera->SetTarget(vector3df(-0.0078f, 0.07f, -0.015f));
	Camera->SetFOV(DEG_TO_RAD(20));
	Camera->SetNearValue(0.1f);
}