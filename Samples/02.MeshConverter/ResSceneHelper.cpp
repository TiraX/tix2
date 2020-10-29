/*
	TiX Engine v2.0 Copyright (C) 2018~2021
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
		: VTSize(0)
		, PageSize(0)
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

		// Load cameras. 
		{
			TJSONNode JCameras = Doc["cameras"];
			Helper.Cameras.clear();
			Helper.Cameras.reserve(JCameras.Size());
			for (int32 c = 0; c < JCameras.Size(); ++c)
			{
				TJSONNode JCam = JCameras[c];
				THeaderCameraInfo Cam;
				Cam.Location = TJSONUtil::JsonArrayToVector3df(JCam["location"]);
				Cam.Target = TJSONUtil::JsonArrayToVector3df(JCam["target"]);
				Cam.Rotate = TJSONUtil::JsonArrayToVector3df(JCam["rotator"]);
				Cam.FOV = JCam["fov"].GetFloat();
				Cam.Aspect = JCam["aspect"].GetFloat();
				Helper.Cameras.push_back(Cam);
			}
		}

		// Load VT region info
		if (!TResSettings::GlobalSettings.VTInfoFile.empty())
		{
			TFile f;
			if (f.Open(TResSettings::GlobalSettings.VTInfoFile, EFA_READ))
			{
				int8* content = ti_new int8[f.GetSize() + 1];
				f.Read(content, f.GetSize(), f.GetSize());
				content[f.GetSize()] = 0;
				f.Close();

				TJSON JsonDoc;
				JsonDoc.Parse(content);

				Helper.VTSize = JsonDoc["vt_size"].GetInt();
				Helper.PageSize = JsonDoc["page_size"].GetInt();

				TJSONNode JRegions = JsonDoc["regions"];

				for (int32 i = 0; i < JRegions.Size(); ++i)
				{
					TJSONNode JRegionObject = JRegions[i];
					TString TextureName = JRegionObject["name"].GetString();

					TJSONNode JRegion = JRegionObject["region"];
					TVTRegionInfo Info;
					Info.X = JRegion[0].GetInt();
					Info.Y = JRegion[1].GetInt();
					Info.W = JRegion[2].GetInt();
					Info.H = JRegion[3].GetInt();
					Helper.VTRegionInfo[TextureName] = Info;
				}
			}
		}

		// Load tiles
		{
			TJSONNode JTileList = Doc["tiles"];

			Helper.AssetSceneTiles.reserve(JTileList.Size());
			for (int32 i = 0; i < JTileList.Size(); ++i)
			{
				TJSONNode JTile = JTileList[i];
				vector2di TilePos = vector2di(JTile[0].GetInt(), JTile[1].GetInt());
				Helper.AssetSceneTiles.push_back(TilePos);
				TI_ASSERT(TMath::Abs(TilePos.X) <= 32760 && TMath::Abs(TilePos.Y) <= 32760);
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

			// Cameras
			Define.NumCameras = (int32)Cameras.size();
			for (const auto& C : Cameras)
			{
				DataStream.Put(&C, sizeof(THeaderCameraInfo));
			}

			// Tile Info
			Define.NumTiles = (int32)AssetSceneTiles.size();

			// Fill Tile Positions
			for (const auto& A : AssetSceneTiles)
			{
				vector2di16 Pos;
				Pos.X = (int16)(A.X);
				Pos.Y = (int16)(A.Y);
				DataStream.Put(&Pos, sizeof(vector2di16));
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
