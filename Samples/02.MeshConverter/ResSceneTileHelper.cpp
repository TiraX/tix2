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
		: StaticMeshesTotal(0)
		, SMSectionsTotal(0)
		, SMInstancesTotal(0)
		, ReflectionCapturesTotal(0)
		, SkeletalMeshTotal(0)
		, SkeletonTotal(0)
		, AnimationTotal(0)
		, SKMActorsTotal(0)
		, EnvLights(0)
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
		const int32 StaticMeshCount = Doc["static_mesh_total"].GetInt();
		const int32 SMInsTotalCount = Doc["sm_instances_total"].GetInt();
		const int32 SMSectionsCount = Doc["sm_sections_total"].GetInt();

		const int32 ReflectionCaptures = Doc["reflection_captures_total"].GetInt();

		const int32 SkeletalMeshTotal = Doc["skeletal_meshes_total"].GetInt();
		const int32 SkeletonTotal = Doc["skeletons_total"].GetInt();
		const int32 AnimationTotal = Doc["anims_total"].GetInt();
		const int32 SKMActorTotal = Doc["skm_actors_total"].GetInt();

		Helper.StaticMeshesTotal = StaticMeshCount;
		Helper.SMSectionsTotal = SMSectionsCount;
		Helper.SMInstancesTotal = SMInsTotalCount;
		Helper.ReflectionCapturesTotal = ReflectionCaptures;
		Helper.SkeletalMeshTotal = SkeletalMeshTotal;
		Helper.SkeletonTotal = SkeletonTotal;
		Helper.AnimationTotal = AnimationTotal;
		Helper.SKMActorsTotal = SKMActorTotal;

		Helper.Position = TJSONUtil::JsonArrayToVector2di(Doc["position"]);
		Helper.BBox = TJSONUtil::JsonArrayToAABBox(Doc["bbox"]);

		// reflection captures
		{
			TJSONNode JReflectionCaptures = Doc["reflection_captures"];
			for (int32 i = 0; i < JReflectionCaptures.Size(); ++i)
			{
				TJSONNode JRC = JReflectionCaptures[i];
				TResEnvLight RC;
				RC.Name = JRC["name"].GetString();
				RC.LinkedCubemap = JRC["linked_cubemap"].GetString();
				RC.Size = JRC["cubemap_size"].GetInt();
				RC.AvgBrightness = JRC["average_brightness"].GetFloat();
				RC.Brightness = JRC["brightness"].GetFloat();
				RC.Position = TJSONUtil::JsonArrayToVector3df(JRC["position"]);

				Helper.EnvLights.push_back(RC);
			}
		}

		// dependency
		{        
			// Load asset list
			TJSONNode JAssetList = Doc["dependency"];
			TJSONNode JAssetTextures = JAssetList["textures"];
			TJSONNode JAssetMaterials = JAssetList["materials"];
			TJSONNode JAssetMaterialInstances = JAssetList["material_instances"];
			TJSONNode JAssetSkeletons = JAssetList["skeletons"];
			TJSONNode JAssetAnims = JAssetList["anims"];
			TJSONNode JAssetSMs = JAssetList["static_meshes"];
			TJSONNode JAssetSKMs = JAssetList["skeletal_meshes"];

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

			Helper.AssetAnims.reserve(JAssetAnims.Size());
			for (int32 i = 0; i < JAssetAnims.Size(); ++i)
			{
				TJSONNode JAnim = JAssetAnims[i];
				Helper.AssetAnims.push_back(JAnim.GetString());
			}

			Helper.AssetSkeletons.reserve(JAssetSkeletons.Size());
			for (int32 i = 0; i < JAssetSkeletons.Size(); ++i)
			{
				TJSONNode JSkeleton = JAssetSkeletons[i];
				Helper.AssetSkeletons.push_back(JSkeleton.GetString());
			}

			Helper.AssetSMs.reserve(JAssetSMs.Size());
			for (int32 i = 0; i < JAssetSMs.Size(); ++i)
			{
				TJSONNode JSM = JAssetSMs[i];
				Helper.AssetSMs.push_back(JSM.GetString());
			}
			Helper.AssetSKMs.reserve(JAssetSKMs.Size());
			for (int32 i = 0; i < JAssetSKMs.Size(); ++i)
			{
				TJSONNode JSKM = JAssetSKMs[i];
				Helper.AssetSKMs.push_back(JSKM.GetString());
			}
		}

		// Static Mesh Instances
		{
			TJSONNode JSMInstanceObjects = Doc["static_mesh_instances"];
			TI_ASSERT(JSMInstanceObjects.IsArray());
			Helper.SMInstances.reserve(SMInsTotalCount);
			Helper.SMInstanceCount.resize(JSMInstanceObjects.Size());
			Helper.SMSections.resize(JSMInstanceObjects.Size());
			for (int32 obj = 0; obj < JSMInstanceObjects.Size(); ++obj)
			{
				TJSONNode JInstanceObj = JSMInstanceObjects[obj];
				TJSONNode JLinkedMesh = JInstanceObj["linked_mesh"];
				TJSONNode JMeshSections = JInstanceObj["mesh_sections"];
				TJSONNode JInstances = JInstanceObj["instances"];

				// Make sure InstanceObject has the same order with dependency-mesh
				TI_ASSERT(JLinkedMesh.GetString() == Helper.AssetSMs[obj]);
				Helper.SMInstanceCount[obj] = JInstances.Size();
				Helper.SMSections[obj] = JMeshSections.GetInt();

				for (int32 ins = 0 ; ins < JInstances.Size(); ++ ins)
				{
					TJSONNode JIns = JInstances[ins];
					TJSONNode JPosition = JIns["position"];
					TJSONNode JRotation = JIns["rotation"];
					TJSONNode JScale = JIns["scale"];

					TResSMInstance Ins;
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
					Helper.SMInstances.push_back(Ins);
				}
			}
		}


		// Skeleton Mesh Actors
		{
			TJSONNode JSKMActorObjects = Doc["skeletal_mesh_actors"];
			TI_ASSERT(JSKMActorObjects.IsArray());
			Helper.SKMActors.reserve(SkeletalMeshTotal);
			for (int32 obj = 0; obj < JSKMActorObjects.Size(); ++obj)
			{
				TJSONNode JActorObj = JSKMActorObjects[obj];
				TJSONNode JLinkedMesh = JActorObj["linked_skm"];
				TJSONNode JLinkedSkeleton = JActorObj["linked_sk"];
				TJSONNode JSKMActors = JActorObj["actors"];

				const int32 SKMIndex = IndexInArray<TString>(JLinkedMesh.GetString(), Helper.AssetSKMs);
				const int32 SKIndex = IndexInArray<TString>(JLinkedSkeleton.GetString(), Helper.AssetSkeletons);

				for (int32 actor = 0; actor < JSKMActors.Size(); ++actor)
				{
					TJSONNode JActor = JSKMActors[actor];

					TJSONNode JAnim = JActor["linked_anim"];
					TJSONNode JPosition = JActor["position"];
					TJSONNode JRotation = JActor["rotation"];
					TJSONNode JScale = JActor["scale"];

					TResSKMActor Actor;
					Actor.SKMIndex = SKMIndex;
					Actor.SKIndex = SKIndex;
					Actor.AnimIndex = IndexInArray<TString>(JAnim.GetString(), Helper.AssetAnims);;
					Actor.Position[0] = JPosition[0].GetFloat();
					Actor.Position[1] = JPosition[1].GetFloat();
					Actor.Position[2] = JPosition[2].GetFloat();
					Actor.Rotation[0] = JRotation[0].GetFloat();
					Actor.Rotation[1] = JRotation[1].GetFloat();
					Actor.Rotation[2] = JRotation[2].GetFloat();
					Actor.Rotation[3] = JRotation[3].GetFloat();
					Actor.Scale[0] = JScale[0].GetFloat();
					Actor.Scale[1] = JScale[1].GetFloat();
					Actor.Scale[2] = JScale[2].GetFloat();
					Helper.SKMActors.push_back(Actor);
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

			SceneTileHeader.NumEnvLights = (int32)EnvLights.size();
			SceneTileHeader.NumTextures = (int32)AssetTextures.size();
			SceneTileHeader.NumMaterials = (int32)AssetMaterials.size();
			SceneTileHeader.NumMaterialInstances = (int32)AssetMaterialInstances.size();
			SceneTileHeader.NumSkeletons = (int32)AssetSkeletons.size();
			SceneTileHeader.NumAnims = (int32)AssetAnims.size();
			SceneTileHeader.NumStaticMeshes = (int32)AssetSMs.size();
			SceneTileHeader.NumSMSections = SMSectionsTotal;
			SceneTileHeader.NumSMInstances = (int32)SMInstances.size();
			SceneTileHeader.NumSkeletalMeshes = (int32)AssetSKMs.size();
			SceneTileHeader.NumSKMActors = (int32)SKMActors.size();

			// reflections
			TI_ASSERT(SceneTileHeader.NumEnvLights == EnvLights.size());
			for (const auto& EnvLight : EnvLights)
			{
				THeaderEnvLight EnvLightHeader;
				EnvLightHeader.NameIndex = AddStringToList(OutStrings, EnvLight.Name);
				EnvLightHeader.LinkedCubemapIndex = AddStringToList(OutStrings, EnvLight.LinkedCubemap);
				EnvLightHeader.Position = EnvLight.Position;

				DataStream.Put(&EnvLightHeader, sizeof(THeaderEnvLight));
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
			for (const auto& A : AssetSkeletons)
			{
				int32 Name = AddStringToList(OutStrings, A);
				DataStream.Put(&Name, sizeof(int32));
			}
			for (const auto& A : AssetAnims)
			{
				int32 Name = AddStringToList(OutStrings, A);
				DataStream.Put(&Name, sizeof(int32));
			}
			for (const auto& A : AssetSMs)
			{
				int32 Name = AddStringToList(OutStrings, A);
				DataStream.Put(&Name, sizeof(int32));
			}
			TI_ASSERT(AssetSMs.size() == SMSections.size());
			for (const auto& A : SMSections)
			{
				int32 Count = A;
				DataStream.Put(&Count, sizeof(int32));
			}
			TI_ASSERT(AssetSMs.size() == SMInstanceCount.size());
			int32 TotalSMInstances = 0;
			for (const auto& A : SMInstanceCount)
			{
				int32 Count = A;
				DataStream.Put(&Count, sizeof(int32));
				TotalSMInstances += Count;
			}
			TI_ASSERT(TotalSMInstances == SceneTileHeader.NumSMInstances);
			for (const auto& A : SMInstances)
			{
				const TResSMInstance& Ins = A;
				DataStream.Put(&Ins, sizeof(TResSMInstance));
			}
			for (const auto& A : AssetSKMs)
			{
				int32 Name = AddStringToList(OutStrings, A);
				DataStream.Put(&Name, sizeof(int32));
			}
			for (const auto& A : SKMActors)
			{
				const TResSKMActor& Actor = A;
				DataStream.Put(&Actor, sizeof(TResSKMActor));
			}

			HeaderStream.Put(&SceneTileHeader, sizeof(THeaderSceneTile));
		}
		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
