/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResMaterialInstanceHelper.h"

namespace tix
{
	TResMaterialInstanceHelper::TResMaterialInstanceHelper()
	{
	}

	TResMaterialInstanceHelper::~TResMaterialInstanceHelper()
	{
	}

	void TResMaterialInstanceHelper::SetMaterialRes(const TString& MaterialName)
	{
		LinkedMaterial = MaterialName;
	}

	void TResMaterialInstanceHelper::AddParameter(const TString& ParamName, int32 Value)
	{
		TVariantValue V;
		V.ValueInt = Value;
		Parameters[ParamName] = V;
	}

	void TResMaterialInstanceHelper::AddParameter(const TString& ParamName, float Value)
	{
		TVariantValue V;
		V.ValueFloat = Value;
		Parameters[ParamName] = V;
	}

	void TResMaterialInstanceHelper::AddParameter(const TString& ParamName, const vector3df& Value)
	{
		TVariantValue V;
		V.ValueVec = Value;
		Parameters[ParamName] = V;
	}

	void TResMaterialInstanceHelper::AddParameter(const TString& ParamName, const quaternion& Value)
	{
		TVariantValue V;
		V.ValueQuat = Value;
		Parameters[ParamName] = V;
	}

	void TResMaterialInstanceHelper::AddParameter(const TString& ParamName, const SColorf& Value)
	{
		TVariantValue V;
		V.ValueClr = Value;
		Parameters[ParamName] = V;
	}
	
	void TResMaterialInstanceHelper::OutputMaterialInstance(TStream& OutStream, TVector<TString>& OutStrings)
	{
		/*
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
		*/
	}
}
