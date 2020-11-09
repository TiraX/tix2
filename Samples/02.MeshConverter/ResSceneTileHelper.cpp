/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResSceneTileHelper.h"

namespace tix
{
	TResSceneTileHelper::TResSceneTileHelper()
		: MeshesTotal(0)
		, MeshSectionsTotal(0)
		, InstancesTotal(0)
		, ReflectionCaptures(0)
	{
	}

	TResSceneTileHelper::~TResSceneTileHelper()
	{
	}

	bool TResSceneTileHelper::LoadSceneTile(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResSceneTileHelper Helper;

		// Instances Name
		TString TileName = Doc["name"].GetString();
		Helper.LevelName = Doc["level"].GetString();
		const int32 MeshCount = Doc["meshes_total"].GetInt();
		const int32 InsTotalCount = Doc["instances_total"].GetInt();
		const int32 MeshSectionsCount = Doc["mesh_sections_total"].GetInt();
		const int32 ReflectionCaptures = Doc["reflection_captures_total"].GetInt();

		Helper.MeshesTotal = MeshCount;
		Helper.MeshSectionsTotal = MeshSectionsCount;
		Helper.InstancesTotal = InsTotalCount;
		Helper.ReflectionCapturesTotal = ReflectionCaptures;

		Helper.Position = TJSONUtil::JsonArrayToVector2di(Doc["position"]);
		Helper.BBox = TJSONUtil::JsonArrayToAABBox(Doc["bbox"]);

		// reflection captures
		{
			TJSONNode JReflectionCaptures = Doc["reflection_captures"];
			for (int32 i = 0; i < JReflectionCaptures.Size(); ++i)
			{
				TJSONNode JRC = JReflectionCaptures[i];
				TResReflectionCapture RC;
				RC.Name = JRC["name"].GetString();
				RC.LinkedCubemap = JRC["linked_cubemap"].GetString();
				RC.Size = JRC["cubemap_size"].GetInt();
				RC.AvgBrightness = JRC["average_brightness"].GetFloat();
				RC.Brightness = JRC["brightness"].GetFloat();
				RC.Position = TJSONUtil::JsonArrayToVector3df(JRC["position"]);

				Helper.ReflectionCaptures.push_back(RC);
			}
		}

		// dependency
		{        
			// Load asset list
			TJSONNode JAssetList = Doc["dependency"];
			TJSONNode JAssetTextures = JAssetList["textures"];
			TJSONNode JAssetMaterials = JAssetList["materials"];
			TJSONNode JAssetMaterialInstances = JAssetList["material_instances"];
			TJSONNode JAssetMeshes = JAssetList["meshes"];

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
		}

		// Instances
		{
			TJSONNode JInstanceObjects = Doc["instances"];
			TI_ASSERT(JInstanceObjects.IsArray());
			Helper.Instances.reserve(InsTotalCount);
			Helper.MeshInstanceCount.resize(JInstanceObjects.Size());
			Helper.MeshSections.resize(JInstanceObjects.Size());
			for (int32 obj = 0; obj < JInstanceObjects.Size(); ++obj)
			{
				TJSONNode JInstanceObj = JInstanceObjects[obj];
				TJSONNode JLinkedMesh = JInstanceObj["linked_mesh"];
				TJSONNode JMeshSections = JInstanceObj["mesh_sections"];
				TJSONNode JInstances = JInstanceObj["instances"];

				// Make sure InstanceObject has the same order with dependency-mesh
				TI_ASSERT(JLinkedMesh.GetString() == Helper.AssetMeshes[obj]);
				Helper.MeshInstanceCount[obj] = JInstances.Size();
				Helper.MeshSections[obj] = JMeshSections.GetInt();

				for (int32 ins = 0 ; ins < JInstances.Size(); ++ ins)
				{
					TJSONNode JIns = JInstances[ins];
					TJSONNode JPosition = JIns["position"];
					TJSONNode JRotation = JIns["rotation"];
					TJSONNode JScale = JIns["scale"];

					TResInstance Ins;
					Ins.Position[0] = JPosition[0].GetFloat();
					Ins.Position[1] = JPosition[1].GetFloat();
					Ins.Position[2] = JPosition[2].GetFloat();
					Ins.Rotation[0] = JRotation[0].GetFloat();
					Ins.Rotation[1] = JRotation[1].GetFloat();
					Ins.Rotation[2] = JRotation[2].GetFloat();
					Ins.Rotation[3] = JRotation[3].GetFloat();
					Ins.Scale[0] = JScale[0].GetFloat();
					Ins.Scale[1] = JScale[1].GetFloat();
					Ins.Scale[2] = JScale[2].GetFloat();
					Helper.Instances.push_back(Ins);
				}
			}
		}

		Helper.OutputTiles(OutStream, OutStrings);
		return true;
	}
	
	void TResSceneTileHelper::OutputTiles(TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResfileChunkHeader ChunkHeader;
		ChunkHeader.ID = TIRES_ID_CHUNK_SCENETILE;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_SCENETILE;
		ChunkHeader.ElementCount = 1;

		TStream HeaderStream, DataStream(1024 * 8);
		for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		{
			// init header
			THeaderSceneTile SceneTileHeader;
			SceneTileHeader.LevelNameIndex = AddStringToList(OutStrings, LevelName);
			SceneTileHeader.Position.X = (int16)Position.X;
			SceneTileHeader.Position.Y = (int16)Position.Y;
			SceneTileHeader.BBox = BBox;

			SceneTileHeader.NumReflectionCaptures = (int32)ReflectionCaptures.size();
			SceneTileHeader.NumTextures = (int32)AssetTextures.size();
			SceneTileHeader.NumMaterials = (int32)AssetMaterials.size();
			SceneTileHeader.NumMaterialInstances = (int32)AssetMaterialInstances.size();
			SceneTileHeader.NumMeshes = (int32)AssetMeshes.size();
			SceneTileHeader.NumMeshSections = MeshSectionsTotal;
			SceneTileHeader.NumInstances = (int32)Instances.size();

			// reflections
			TI_ASSERT(SceneTileHeader.NumReflectionCaptures == ReflectionCaptures.size());
			for (const auto& RC : ReflectionCaptures)
			{
				THeaderSceneReflectionCapture RCHeader;
				RCHeader.NameIndex = AddStringToList(OutStrings, RC.Name);
				RCHeader.LinkedCubemapIndex = AddStringToList(OutStrings, RC.LinkedCubemap);
				RCHeader.Position = RC.Position;

				DataStream.Put(&RCHeader, sizeof(THeaderSceneReflectionCapture));
			}

			// dependencies
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
			TI_ASSERT(AssetMeshes.size() == MeshSections.size());
			for (const auto& A : MeshSections)
			{
				int32 Count = A;
				DataStream.Put(&Count, sizeof(int32));
			}
			TI_ASSERT(AssetMeshes.size() == MeshInstanceCount.size());
			int32 TotalInstances = 0;
			for (const auto& A : MeshInstanceCount)
			{
				int32 Count = A;
				DataStream.Put(&Count, sizeof(int32));
				TotalInstances += Count;
			}
			TI_ASSERT(TotalInstances == SceneTileHeader.NumInstances);
			for (const auto& A : Instances)
			{
				const TResInstance& Ins = A;
				DataStream.Put(&Ins, sizeof(TResInstance));
			}

			HeaderStream.Put(&SceneTileHeader, sizeof(THeaderSceneTile));
		}
		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
