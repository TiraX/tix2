/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResSceneHelper.h"
#include "rapidjson/document.h"

using namespace rapidjson;

namespace tix
{
	TResSceneHelper::TResSceneHelper()
	{
	}

	TResSceneHelper::~TResSceneHelper()
	{
	}

	void TResSceneHelper::LoadScene(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResSceneHelper Helper;

		// Map Name
		TJSONNode JMapName = Doc["name"];
		Helper.MapName = JMapName.GetString();

		// Load environments. include sun light, fog, etc
		{
			TJSONNode JEnv = Doc["environment"];

			// Sun light
			TJSONNode JSunLight = JEnv["sun_light"];
			Helper.Environment.SunLight.Direction = TJSONUtil::JsonArrayToVector3df(JSunLight["direction"]);
			Helper.Environment.SunLight.Color = TJSONUtil::JsonArrayToSColorf(JSunLight["color"]);
			Helper.Environment.SunLight.Intensity = JSunLight["intensity"].GetFloat();
		}

		// Load asset list
		{
			TJSONNode JAssetList = Doc["dependency"];
			TJSONNode JAssetTextures = JAssetList["textures"];
			TJSONNode JAssetMaterials = JAssetList["materials"];
			TJSONNode JAssetMaterialInstances = JAssetList["material_instances"];
			TJSONNode JAssetMeshes = JAssetList["meshes"];
			TJSONNode JAssetInstances = Doc["instances"];

			Helper.AssetTextures.reserve(JAssetTextures.Size());
			for (int32 i = 0; i < JAssetTextures.Size(); ++i)
			{
				TJSONNode JTexture = JAssetTextures[i];
				Helper.AssetTextures.push_back(JTexture.GetString());
			}
			Helper.AssetMaterials.reserve(JAssetMaterials.Size());
			for (int32 i = 0; i < JAssetMaterials.Size(); ++i)
			{
				TJSONNode JMaterial = JAssetMaterials[i];
				Helper.AssetMaterials.push_back(JMaterial.GetString());
			}
			Helper.AssetMaterialInstances.reserve(JAssetMaterialInstances.Size());
			for (int32 i = 0; i < JAssetMaterialInstances.Size(); ++i)
			{
				TJSONNode JMI = JAssetMaterialInstances[i];
				Helper.AssetMaterialInstances.push_back(JMI.GetString());
			}
			Helper.AssetMeshes.reserve(JAssetMeshes.Size());
			for (int32 i = 0; i < JAssetMeshes.Size(); ++i)
			{
				TJSONNode JMesh = JAssetMeshes[i];
				Helper.AssetMeshes.push_back(JMesh.GetString());
			}
			Helper.AssetInstances.reserve(JAssetInstances.Size());
			for (int32 i = 0; i < JAssetInstances.Size(); ++i)
			{
				TJSONNode JInstances = JAssetInstances[i];
				Helper.AssetInstances.push_back(JInstances.GetString());
			}
		}

		Helper.OutputScene(OutStream, OutStrings);
	}
	
	void TResSceneHelper::OutputScene(TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResfileChunkHeader ChunkHeader;
		ChunkHeader.ID = TIRES_ID_CHUNK_SCENE;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_SCENE;
		ChunkHeader.ElementCount = 1;

		TStream HeaderStream, DataStream(1024 * 8);
		for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		{
			THeaderScene Define;
			Define.NameIndex = AddStringToList(OutStrings, MapName);

			// Environment Info
			Define.MainLightDirection = Environment.SunLight.Direction;
			Define.MainLightColor = Environment.SunLight.Color;
			Define.MainLightIntensity = Environment.SunLight.Intensity;

			// Assets Info
			Define.NumTextures = (int32)AssetTextures.size();
			Define.NumMaterials = (int32)AssetMaterials.size();
			Define.NumMaterialInstances = (int32)AssetMaterialInstances.size();
			Define.NumMeshes = (int32)AssetMeshes.size();
			Define.NumInstances = (int32)AssetInstances.size();

			// Fill Assets Names
			for (const auto& A : AssetTextures)
			{
				int32 Name = AddStringToList(OutStrings, A);
				DataStream.Put(&Name, sizeof(int32));
			}
			for (const auto& A : AssetMaterials)
			{
				int32 Name = AddStringToList(OutStrings, A);
				DataStream.Put(&Name, sizeof(int32));
			}
			for (const auto& A : AssetMaterialInstances)
			{
				int32 Name = AddStringToList(OutStrings, A);
				DataStream.Put(&Name, sizeof(int32));
			}
			for (const auto& A : AssetMeshes)
			{
				int32 Name = AddStringToList(OutStrings, A);
				DataStream.Put(&Name, sizeof(int32));
			}
			for (const auto& A : AssetInstances)
			{
				int32 Name = AddStringToList(OutStrings, A);
				DataStream.Put(&Name, sizeof(int32));
			}

			TI_ASSERT(AssetMeshes.size() == AssetInstances.size());
			
			// Save header
			HeaderStream.Put(&Define, sizeof(THeaderScene));
		}

		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
