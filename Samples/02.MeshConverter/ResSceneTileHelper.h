/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TResInstance
	{
		vector3df Position;
		quaternion Rotation;
		vector3df Scale;
	};
	class TResSceneTileHelper
	{
	public:
		TResSceneTileHelper();
		~TResSceneTileHelper();

		static bool LoadSceneTile(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings);
		void OutputTiles(TStream& OutStream, TVector<TString>& OutStrings);

		TString LevelName;
		vector2di Position;
		aabbox3df BBox;
		
		TVector<TString> AssetTextures;
		TVector<TString> AssetMaterialInstances;
		TVector<TString> AssetMaterials;
		TVector<TString> AssetMeshes;
		TVector<int32> MeshInstanceCount;
		TVector<int32> MeshSections;
		TVector<TResInstance> Instances;
	};
}