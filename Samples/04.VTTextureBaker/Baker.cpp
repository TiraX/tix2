/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "Baker.h"

namespace tix
{
	TVTTextureBaker::TVTTextureBaker()
		: VTRegion(VTSize, PPSize)
	{
	}

	TVTTextureBaker::~TVTTextureBaker()
	{
	}

	void TVTTextureBaker::LoadTextureFiles(const TString& SceneFileName)
	{
		TFile f;
		if (!f.Open(SceneFileName, EFA_READ))
		{
			printf("Error : Failed to open file : %s\n", SceneFileName.c_str());
			return;
		}

		int8* content = ti_new int8[f.GetSize() + 1];
		f.Read(content, f.GetSize(), f.GetSize());
		content[f.GetSize()] = 0;
		f.Close();

		TJSON JsonDoc;
		JsonDoc.Parse(content);
		ti_delete content;

		TJSONNode JAssetList = JsonDoc["dependency"];
		TJSONNode JAssetTextures = JAssetList["textures"];

		// Load texture names
		TVector<TString> TextureNames;
		TextureNames.reserve(JAssetTextures.Size());
		for (int32 i = 0; i < JAssetTextures.Size(); ++i)
		{
			TJSONNode JTexture = JAssetTextures[i];
			TextureNames.push_back(JTexture.GetString());
		}

		// Load texture basic infos
		TextureInfos.reserve(TextureNames.size());
		for (const auto& T : TextureNames)
		{
			size_t Pos = T.find(".tasset");
			TString TJSName = T.substr(0, Pos) + ".tjs";

			TFile TexTjsFile;
			if (!TexTjsFile.Open(TJSName, EFA_READ))
			{
				printf("Error: Failed to open %s\n", TJSName.c_str());
				continue;
			}

			int8* TexJson = ti_new int8[TexTjsFile.GetSize() + 1];
			TexTjsFile.Read(TexJson, TexTjsFile.GetSize(), TexTjsFile.GetSize());
			TexJson[TexTjsFile.GetSize()] = 0;
			TexTjsFile.Close();

			TJSON TexJsonDoc;
			TexJsonDoc.Parse(TexJson);
			ti_delete TexJson;

			TVTTextureBasicInfo Info;
			Info.AddressMode = GetAddressMode(TexJsonDoc["address_mode"].GetString());
			Info.Srgb = TexJsonDoc["srgb"].GetInt();
			Info.LodBias = TexJsonDoc["lod_bias"].GetInt();

			Pos = T.rfind('/');
			Info.Name = T.substr(0, Pos + 1) + TexJsonDoc["source"].GetString();

			Info.Size = TImage::LoadImageTGADimension(Info.Name);
			TextureInfos.push_back(Info);
		}

		for (const auto& Info : TextureInfos)
		{
			uint32 RegionIndex;
			TRegion::TRegionDesc* Region = VTRegion.FindAvailbleRegion(Info.Size.X, Info.Size.Y, &RegionIndex);

			//int32 x = RegionIndex % ITSize;
			//int32 y = RegionIndex / ITSize;
			//InPrimitive->SetUVTransform(x * UVInv, y * UVInv, Region->XCount * UVInv, Region->YCount * UVInv);
		}
	}
}
