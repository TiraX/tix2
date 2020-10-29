/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResTextureHelper.h"
#include "ResMultiThreadTask.h"

namespace tix
{
	TResTextureHelper::TResTextureHelper()
	{
	}

	TResTextureHelper::~TResTextureHelper()
	{
		for (auto T : Textures)
		{
			ti_delete T;
		}
		Textures.clear();
	}

	void TResTextureHelper::AddTexture(TResTextureDefine* Texture)
	{
		Textures.push_back(Texture);
	}

	bool TResTextureHelper::LoadTextureFile(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TString Name = Doc["name"].GetString();
		//int32 Version = Doc["version"].GetInt();

		TResTextureSourceInfo SrcInfo;
		SrcInfo.SRGB = Doc["srgb"].GetInt();
		SrcInfo.AddressMode = GetAddressMode(Doc["address_mode"].GetString());
		SrcInfo.TargetFormat = GetPixelFormat(Doc["target_format"].GetString());

		SrcInfo.LodBias = Doc["lod_bias"].GetInt();
		SrcInfo.IsNormalmap = Doc["is_normalmap"].GetInt();
		SrcInfo.IsIBL = Doc["ibl"].GetInt();
		SrcInfo.HasMips = Doc["has_mips"].GetInt();
		SrcInfo.TextureSource = Doc["source"].GetString();

		TString ExtName = GetExtName(SrcInfo.TextureSource);
		// Load Texture By Name
		TResTextureDefine * SrcImage = nullptr;
		TString SrcImageType;
		if (ExtName == "dds")
		{
			SrcImage = TResTextureHelper::LoadDdsFile(SrcInfo);
			SrcImageType = "DDS";
		}
		else if (ExtName == "tga")
		{
			SrcImage = TResTextureHelper::LoadTgaFile(SrcInfo);
			SrcImageType = "TGA";
		}
		else if (ExtName == "hdr")
		{
			SrcImage = TResTextureHelper::LoadHdrFile(SrcInfo);
			SrcImageType = "HDR";
		}
		else
		{
			_LOG(Error, "Unknown texture format : %s\n", SrcInfo.TextureSource.c_str());
			return false;
		}

		if (SrcInfo.IsIBL)
		{
			TI_ASSERT(SrcImage->Desc.Type == ETT_TEXTURE_CUBE);
			TI_ASSERT(SrcImage->Desc.Format == EPF_RGBA32F || SrcImage->Desc.Format == EPF_RGBA16F);

			// Do pbr ibl filter
			TI_TODO("IBL pbr filter.");
		}

		// Convert to target FORMAT
		TResTextureDefine* TextureOutput = nullptr;
		if (SrcImageType == "DDS")
		{
#if defined (TI_PLATFORM_WIN32)
			// Win32 Platform need DDS texture
			TextureOutput = SrcImage;
#elif defined (TI_PLATFORM_IOS)
			// iOS Platform need ASTC texture
			_LOG(Error, "DDS to ASTC not implemented yet.\n");
			return false;
#endif
		}
		else if (SrcImageType == "HDR")
		{
			if (SrcImage->Desc.Format == EPF_RGBA32F)
			{
				// Convert to RGBA16F
				TextureOutput = TResTextureHelper::Convert32FTo16F(SrcImage);
			}
			else
			{
				TI_ASSERT(SrcImage->Desc.Format == EPF_RGBA16F);
				TextureOutput = SrcImage;
			}
		}
		else
		{
#if defined (TI_PLATFORM_WIN32)
			// Win32 Platform need DDS texture
			TextureOutput = TResTextureHelper::ConvertToDds(SrcImage);
#elif defined (TI_PLATFORM_IOS)
			// iOS Platform need ASTC texture
			TextureOutput = TResTextureHelper::ConvertToAstc(SrcImage);
#endif
			ti_delete SrcImage;
		}

		if (TextureOutput != nullptr)
		{
			TI_ASSERT(SrcInfo.LodBias < (int32)TextureOutput->Desc.Mips);

			TextureOutput->LodBias = SrcInfo.LodBias;
			TextureOutput->Desc.AddressMode = SrcInfo.AddressMode;
			TextureOutput->Desc.SRGB = SrcInfo.SRGB;

			TResTextureHelper Helper;
			Helper.AddTexture(TextureOutput);
			Helper.OutputTexture(OutStream, OutStrings);

			return true;
		}
		else
		{
			_LOG(Error, "Can not load texture : %s.\n", SrcInfo.TextureSource.c_str());
			return false;
		}
	}
	
	void TResTextureHelper::OutputTexture(TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResfileChunkHeader ChunkHeader;
		ChunkHeader.ID = TIRES_ID_CHUNK_TEXTURE;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_TEXTURE;
		ChunkHeader.ElementCount = (int32)Textures.size();

		TStream HeaderStream, DataStream(1024 * 8);
		for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		{
			TResTextureDefine* Define = Textures[t];

			int32 Faces = 1;
			if (Define->Desc.Type == ETT_TEXTURE_CUBE)
			{
				Faces = 6;
			}

			// init header
			THeaderTexture TextureHeader;
			memset(&TextureHeader, 0, sizeof(THeaderTexture));

			TextureHeader.StrId_Name = AddStringToList(OutStrings, Define->Name);
			TextureHeader.Format = Define->Desc.Format;
			TextureHeader.Width = Define->Desc.Width;
			TextureHeader.Height = Define->Desc.Height;
			TextureHeader.Type = Define->Desc.Type;
			TextureHeader.AddressMode = Define->Desc.AddressMode;
			TextureHeader.SRGB = Define->Desc.SRGB;
			TextureHeader.Mips = Define->Desc.Mips;
			TextureHeader.Surfaces = (uint32)Define->ImageSurfaces.size() * Define->Desc.Mips;
			TI_ASSERT(Define->Desc.Mips == Define->ImageSurfaces[0]->GetMipmapCount());

			HeaderStream.Put(&TextureHeader, sizeof(THeaderTexture));
			FillZero4(HeaderStream);

			const TVector<TImage*>& Surfaces = Define->ImageSurfaces;
			for (int32 Face = 0; Face < Faces; ++Face)
			{
				for (uint32 Mip = 0; Mip < Define->Desc.Mips; ++Mip)
				{
					const TImage::TSurfaceData& Surface = Surfaces[Face]->GetMipmap(Mip);
					int32 DataLength = TMath::Align4(Surface.Data.GetLength());
					DataStream.Put(&Surface.W, sizeof(int32));
					DataStream.Put(&Surface.H, sizeof(int32));
					DataStream.Put(&Surface.RowPitch, sizeof(int32));
					DataStream.Put(&DataLength, sizeof(int32));

					DataStream.Put(Surface.Data.GetBuffer(), Surface.Data.GetLength());
					FillZero4(DataStream);
				}
			}
		}
		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
