/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResTextureHelper.h"

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
		SrcInfo.HasMips = Doc["has_mips"].GetInt();
		SrcInfo.TextureSource = Doc["source"].GetString();

		TString ExtName = GetExtName(SrcInfo.TextureSource);
		// Load Texture By Name
		TResTextureDefine * Texture = nullptr;
		if (ExtName == "dds")
		{
			Texture = TResTextureHelper::LoadDdsFile(SrcInfo);
#if defined (TI_PLATFORM_IOS)
			// Convert to ASTC format for iOS cook
			TResTextureDefine* AstcTexture = TResTextureHelper::ConvertDdsToAstc(Texture, TextureSource, LodBias, TargetFormat);
			ti_delete Texture;
			Texture = AstcTexture;
#endif
		}
		else if (ExtName == "tga")
		{
#if defined (TI_PLATFORM_WIN32)
			// Convert to DXT format for Win32
			Texture = TResTextureHelper::LoadTgaToDds(SrcInfo);
#elif defined (TI_PLATFORM_IOS)
			// Convert to ASTC format for iOS
			Texture = TResTextureHelper::LoadTgaToAstc(SrcInfo);
#endif
		}
		else
		{
			printf("Error : Unknown texture format : %s\n", SrcInfo.TextureSource.c_str());
			return false;
		}

//#if defined (TI_PLATFORM_WIN32)
//		TResTextureDefine* Texture = TResTextureHelper::LoadDdsFile(TextureSource, LodBias);
//#elif defined (TI_PLATFORM_IOS)
//       TResTextureDefine* Texture = TResTextureHelper::LoadAstcFile(TextureSource, LodBias, TargetFormat);
//#endif
		if (Texture != nullptr)
		{
			TI_ASSERT(SrcInfo.LodBias < (int32)Texture->Desc.Mips);

			Texture->LodBias = SrcInfo.LodBias;
			Texture->Desc.AddressMode = SrcInfo.AddressMode;
			Texture->Desc.SRGB = SrcInfo.SRGB;

			TResTextureHelper Helper;
			Helper.AddTexture(Texture);
			Helper.OutputTexture(OutStream, OutStrings);

			return true;
		}
		else
		{
			printf("Can not load texture : %s.\n", SrcInfo.TextureSource.c_str());
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
			TextureHeader.Surfaces = (uint32)Define->Surfaces.size();

			HeaderStream.Put(&TextureHeader, sizeof(THeaderTexture));
			FillZero4(HeaderStream);

			const TVector<TResSurfaceData>& Surfaces = Define->Surfaces;
			for (int32 f = 0; f < Faces; ++f)
			{
				for (uint32 mip = 0; mip < Define->Desc.Mips; ++mip)
				{
					int32 SurfaceIndex = f * Define->Desc.Mips + mip;

					const TResSurfaceData& Surface = Surfaces[SurfaceIndex];
					int32 DataLength = ti_align4(Surface.Data.GetLength());
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
