/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FScene.h"
#include "FSceneLights.h"

namespace tix
{
	FScene::FScene()
		: SceneLights(nullptr)
		, SceneFlags(0)
	{
		SceneLights = ti_new FSceneLights;

		// Reserve memory for containers
		SceneTiles.reserve(128);
	}

	FScene::~FScene()
	{
		SAFE_DELETE(SceneLights);
	}

	void FScene::SetViewProjection(const FViewProjectionInfo& Info)
	{
		TI_ASSERT(IsRenderThread());
		ViewProjection = Info;
		SetSceneFlag(ViewProjectionDirty);
	}

	void FScene::SetEnvironmentInfo(const FEnvironmentInfo& Info)
	{
		TI_ASSERT(IsRenderThread());
		EnvInfo = Info;
		SetSceneFlag(EnvironmentDirty);
	}

	void FScene::InitRenderFrame()
	{
		PrepareViewUniforms();
	}

	// From UE4
	void SetupSkyIrradianceEnvironmentMapConstantsFromSkyIrradiance(FFloat4* OutSkyIrradianceEnvironmentMap, const FSHVectorRGB3& SkyIrradiance)
	{
		const float SqrtPI = TMath::Sqrt(PI);  
		const float Coefficient0 = 1.0f / (2 * SqrtPI);
		const float Coefficient1 = TMath::Sqrt(3) / (3 * SqrtPI);
		const float Coefficient2 = TMath::Sqrt(15) / (8 * SqrtPI);
		const float Coefficient3 = TMath::Sqrt(5) / (16 * SqrtPI);
		const float Coefficient4 = .5f * Coefficient2;

		// Pack the SH coefficients in a way that makes applying the lighting use the least shader instructions
		// This has the diffuse convolution coefficients baked in
		// See "Stupid Spherical Harmonics (SH) Tricks"
		OutSkyIrradianceEnvironmentMap[0].X = -Coefficient1 * SkyIrradiance.R.V[3];
		OutSkyIrradianceEnvironmentMap[0].Y = -Coefficient1 * SkyIrradiance.R.V[1];
		OutSkyIrradianceEnvironmentMap[0].Z = Coefficient1 * SkyIrradiance.R.V[2];
		OutSkyIrradianceEnvironmentMap[0].W = Coefficient0 * SkyIrradiance.R.V[0] - Coefficient3 * SkyIrradiance.R.V[6];

		OutSkyIrradianceEnvironmentMap[1].X = -Coefficient1 * SkyIrradiance.G.V[3];
		OutSkyIrradianceEnvironmentMap[1].Y = -Coefficient1 * SkyIrradiance.G.V[1];
		OutSkyIrradianceEnvironmentMap[1].Z = Coefficient1 * SkyIrradiance.G.V[2];
		OutSkyIrradianceEnvironmentMap[1].W = Coefficient0 * SkyIrradiance.G.V[0] - Coefficient3 * SkyIrradiance.G.V[6];

		OutSkyIrradianceEnvironmentMap[2].X = -Coefficient1 * SkyIrradiance.B.V[3];
		OutSkyIrradianceEnvironmentMap[2].Y = -Coefficient1 * SkyIrradiance.B.V[1];
		OutSkyIrradianceEnvironmentMap[2].Z = Coefficient1 * SkyIrradiance.B.V[2];
		OutSkyIrradianceEnvironmentMap[2].W = Coefficient0 * SkyIrradiance.B.V[0] - Coefficient3 * SkyIrradiance.B.V[6];

		OutSkyIrradianceEnvironmentMap[3].X = Coefficient2 * SkyIrradiance.R.V[4];
		OutSkyIrradianceEnvironmentMap[3].Y = -Coefficient2 * SkyIrradiance.R.V[5];
		OutSkyIrradianceEnvironmentMap[3].Z = 3 * Coefficient3 * SkyIrradiance.R.V[6];
		OutSkyIrradianceEnvironmentMap[3].W = -Coefficient2 * SkyIrradiance.R.V[7];

		OutSkyIrradianceEnvironmentMap[4].X = Coefficient2 * SkyIrradiance.G.V[4];
		OutSkyIrradianceEnvironmentMap[4].Y = -Coefficient2 * SkyIrradiance.G.V[5];
		OutSkyIrradianceEnvironmentMap[4].Z = 3 * Coefficient3 * SkyIrradiance.G.V[6];
		OutSkyIrradianceEnvironmentMap[4].W = -Coefficient2 * SkyIrradiance.G.V[7];

		OutSkyIrradianceEnvironmentMap[5].X = Coefficient2 * SkyIrradiance.B.V[4];
		OutSkyIrradianceEnvironmentMap[5].Y = -Coefficient2 * SkyIrradiance.B.V[5];
		OutSkyIrradianceEnvironmentMap[5].Z = 3 * Coefficient3 * SkyIrradiance.B.V[6];
		OutSkyIrradianceEnvironmentMap[5].W = -Coefficient2 * SkyIrradiance.B.V[7];

		OutSkyIrradianceEnvironmentMap[6].X = Coefficient4 * SkyIrradiance.R.V[8];
		OutSkyIrradianceEnvironmentMap[6].Y = Coefficient4 * SkyIrradiance.G.V[8];
		OutSkyIrradianceEnvironmentMap[6].Z = Coefficient4 * SkyIrradiance.B.V[8];
		OutSkyIrradianceEnvironmentMap[6].W = 1;
	}

	void FScene::PrepareViewUniforms()
	{
		if (HasSceneFlag(FScene::ViewUniformDirty))
		{
			// Always make a new View uniform buffer for on-the-fly rendering
			ViewUniformBuffer = ti_new FViewUniformBuffer();

			const FViewProjectionInfo& VPInfo = GetViewProjection();
			ViewUniformBuffer->UniformBufferData[0].ViewProjection = VPInfo.MatProj * VPInfo.MatView;
			ViewUniformBuffer->UniformBufferData[0].ViewDir = VPInfo.CamDir;
			ViewUniformBuffer->UniformBufferData[0].ViewPos = VPInfo.CamPos;

			ViewUniformBuffer->UniformBufferData[0].MainLightDirection = -EnvInfo.MainLightDirection;
			ViewUniformBuffer->UniformBufferData[0].MainLightColor = EnvInfo.MainLightColor * EnvInfo.MainLightIntensity;

			SetupSkyIrradianceEnvironmentMapConstantsFromSkyIrradiance(ViewUniformBuffer->UniformBufferData[0].SkyIrradiance, EnvInfo.SkyIrradiance);

			ViewUniformBuffer->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
		}
	}

	void FScene::AddSceneTileInfo(FSceneTileResourcePtr SceneTileResource)
	{
		if (SceneTileResource != nullptr)
		{
			SceneTiles[SceneTileResource->GetTilePosition()] = SceneTileResource;

			// Mark flag scene tile dirty
			SetSceneFlag(SceneTileDirty);
		}
	}

	void FScene::RemoveSceneTileInfo(FSceneTileResourcePtr SceneTileResource)
	{
		TI_TODO("Add scene tile unregister.");
		TI_ASSERT(0);
	}

	void FScene::AddSceneMeshBuffer(FMeshBufferPtr InMesh, FMeshBufferPtr InOccludeMesh, FUniformBufferPtr InClusterData)
	{
		THMap<FMeshBufferPtr, FSceneMeshInfo>::iterator It = SceneMeshes.find(InMesh);
		if (It != SceneMeshes.end())
		{
			TI_ASSERT(It->second.OccludeMesh == InOccludeMesh && It->second.ClusterData == InClusterData);
			++It->second.References;
		}
		else
		{
			SceneMeshes[InMesh] = FSceneMeshInfo(1, InOccludeMesh, InClusterData);
		}
	}

	void FScene::RemoveSceneMeshBuffer(FMeshBufferPtr InMesh)
	{
		THMap<FMeshBufferPtr, FSceneMeshInfo>::iterator It = SceneMeshes.find(InMesh);
		TI_ASSERT(It != SceneMeshes.end() && It->second.References > 0);

		--It->second.References;
		if (It->second.References == 0)
		{
			SceneMeshes.erase(It);
		}
	}

	void FScene::AddEnvLight(FTexturePtr CubeTexture, const vector3df& Position)
	{
		TI_TODO("Create quad-tree to fast find nearest Env Light.");
		EnvLight = ti_new FEnvLight(CubeTexture, Position);
	}

	void FScene::RemoveEnvLight(FEnvLightPtr InEnvLight)
	{
		TI_ASSERT(0);
	}

}
