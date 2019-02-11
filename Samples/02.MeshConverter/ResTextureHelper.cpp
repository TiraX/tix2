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

		//TString TType = Doc["texture_type"].GetString();
		//E_TEXTURE_TYPE TextureType = GetTextureType(TType);

		int32 SRgb = Doc["srgb"].GetInt();
		TString AddressModeStr = Doc["address_mode"].GetString();
		E_TEXTURE_ADDRESS_MODE AddressMode = GetAddressMode(AddressModeStr);
		E_PIXEL_FORMAT TargetFormat = GetPixelFormat(Doc["target_format"].GetString());

		uint32 LodBias = Doc["lod_bias"].GetInt();

		TString TextureSource = Doc["source"].GetString();

#if defined (TI_PLATFORM_WIN32)
		TResTextureDefine* Texture = TResTextureHelper::LoadDdsFile(TextureSource, LodBias);
#elif defined (TI_PLATFORM_IOS)
        TResTextureDefine* Texture = TResTextureHelper::LoadAstcFile(TextureSource, LodBias);
#endif
		if (Texture != nullptr)
		{
			TI_ASSERT(LodBias < Texture->Desc.Mips);

			Texture->LodBias = LodBias;
			Texture->Desc.AddressMode = AddressMode;
			Texture->Desc.SRGB = SRgb;

			TResTextureHelper Helper;
			Helper.AddTexture(Texture);
			Helper.OutputTexture(OutStream, OutStrings);

			return true;
		}
		else
		{
			printf("Can not load texture : %s.\n", TextureSource.c_str());
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
