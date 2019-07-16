/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TSceneMeshInstance
	{
		vector3df Position;
		quaternion Rotation;
		vector3df Scale;

		TSceneMeshInstance()
			: Scale(1.f, 1.f, 1.f)
		{}
	};
	struct TSceneMeshInstances
	{
		TString MeshName;
		TVector<TSceneMeshInstance> Instances;
	};

	struct TSceneEnvSunLight
	{
		vector3df Direction;
		SColorf Color;
		float Intensity;

		TSceneEnvSunLight()
			: Direction(0, 0, -1.f)
			, Color(1.f, 1.f, 1.f, 1.f)
			, Intensity(3.14f)
		{}
	};

	struct TSceneEnvironment
	{
		TSceneEnvSunLight SunLight;
	};

	class TResSceneHelper
	{
	public:
		TResSceneHelper();
		~TResSceneHelper();

		static void LoadScene(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings);
		void OutputScene(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
		TString MapName;
		TSceneEnvironment Environment;
		TVector<THeaderCameraInfo> Cameras;

		int32 VTSize;
		int32 PageSize;

		TVector<TString> AssetTextures;
		TVector<TString> AssetMaterialInstances;
		TVector<TString> AssetMaterials;
		TVector<TString> AssetMeshes;
		TVector<TString> AssetInstances;
		THMap<TString, TVTRegionInfo> VTRegionInfo;
	};
}