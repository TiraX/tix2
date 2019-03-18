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

		// Load scene meshes and instances
		{
			TJSONNode JSceneMeshes = Doc["scene"];
			TI_ASSERT(JSceneMeshes.IsArray());
			Helper.Meshes.resize(JSceneMeshes.Size());
			for (int32 m = 0; m < JSceneMeshes.Size(); ++m)
			{
				TJSONNode JMesh = JSceneMeshes[m];
				Helper.Meshes[m].MeshName = JMesh["name"].GetString();

				TJSONNode JInstances = JMesh["instances"];
				Helper.Meshes[m].Instances.resize(JInstances.Size());
				TVector<TSceneMeshInstance>& Instances = Helper.Meshes[m].Instances;
				for (int32 ins = 0; ins < JInstances.Size(); ++ins)
				{
					TJSONNode JInstance = JInstances[ins];
					Instances[ins].Position = TJSONUtil::JsonArrayToVector3df(JInstance["position"]);
					Instances[ins].Rotation = TJSONUtil::JsonArrayToQuaternion(JInstance["rotation"]);
					Instances[ins].Scale = TJSONUtil::JsonArrayToVector3df(JInstance["scale"]);
				}
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
			Define.SunLightDirection = Environment.SunLight.Direction;
			Define.SunLightColor = Environment.SunLight.Color;
			Define.SunLightIntensity = Environment.SunLight.Intensity;

			// Meshes and Instances Info
			Define.Meshes = (int32)Meshes.size();
			// Calc total instances
			int32 TotalInstances = 0;
			for (uint32 m = 0 ; m < Meshes.size(); ++ m)
			{
				TotalInstances += (int32)Meshes[m].Instances.size();
			}
			Define.Instances = TotalInstances;

			// Fill Meshes Data
			for (uint32 m = 0 ; m < Meshes.size(); ++ m)
			{
				THeaderSceneMesh SceneMeshDefine;
				SceneMeshDefine.MeshNameIndex = AddStringToList(OutStrings, Meshes[m].MeshName);
				SceneMeshDefine.MeshInstances = (int32)Meshes[m].Instances.size();
				DataStream.Put(&SceneMeshDefine, sizeof(THeaderSceneMesh));
			}
			// Fill Mesh Instances Data
			for (uint32 m = 0; m < Meshes.size(); ++m)
			{
				TVector<TSceneMeshInstance>& Instances = Meshes[m].Instances;
				for (int32 ins = 0 ; ins < Instances.size(); ++ ins)
				{
					THeaderSceneMeshInstance SceneMeshInstanceDefine;
					SceneMeshInstanceDefine.Position = Instances[ins].Position;
					SceneMeshInstanceDefine.Rotation = Instances[ins].Rotation;
					SceneMeshInstanceDefine.Scale = Instances[ins].Scale;
					DataStream.Put(&SceneMeshInstanceDefine, sizeof(THeaderSceneMeshInstance));
				}
			}
			
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
