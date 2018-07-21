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

			// init header
			THeaderTexture TextureHeader;
			TextureHeader.StrId_Name = AddStringToList(OutStrings, Define->Name);
			TextureHeader.Desc = Define->Desc;

			HeaderStream.Put(&TextureHeader, sizeof(THeaderTexture));
			FillZero8(HeaderStream);

			const TVector<TResSurfaceData>& Surfaces = Define->Surfaces;
			for (const auto& Surface : Surfaces)
			{
				int32 DataLength = ti_align4(Surface.Data.GetLength());
				DataStream.Put(&Surface.W, sizeof(int32));
				DataStream.Put(&Surface.H, sizeof(int32));
				DataStream.Put(&Surface.RowPitch, sizeof(int32));
				DataStream.Put(&DataLength, sizeof(int32));

				DataStream.Put(Surface.Data.GetBuffer(), Surface.Data.GetLength());
				FillZero4(DataStream);
			}
		}
		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero8(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
