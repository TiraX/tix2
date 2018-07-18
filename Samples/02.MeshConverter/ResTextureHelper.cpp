/*
	TiX Engine v2.0 Copyright (C) 2018
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
	}

	void TResTextureHelper::AddTexture(const TString& Name, const TString& Path, TTexturePtr Texture)
	{
		TTextureDefine Define;
		Define.Name = Name;
		Define.Path = Path;
		Define.TextureRes = Texture;

		Textures.push_back(Define);
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
			TTextureDefine Define = Textures[t];

			// init header
			THeaderTexture TextureHeader;
			TextureHeader.StrId_Name = AddStringToList(OutStrings, Define.Name);
			TextureHeader.Desc = Define.TextureRes->Desc;

			HeaderStream.Put(&TextureHeader, sizeof(THeaderTexture));
			FillZero8(HeaderStream);

			const TVector<TTexture::TSurface*>& Surfaces = Define.TextureRes->GetSurfaces();
			for (auto* Surface : Surfaces)
			{
				TI_ASSERT(Surface->DataSize % 4 == 0);
				DataStream.Put(&Surface->Width, sizeof(uint32));
				DataStream.Put(&Surface->Height, sizeof(uint32));
				DataStream.Put(&Surface->DataSize, sizeof(uint32));
				DataStream.Put(&Surface->Pitch, sizeof(uint32));

				DataStream.Put(Surface->Data, Surface->DataSize);
				FillZero8(DataStream);
			}
		}
		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero8(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
