/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TTextureLoader.h"

namespace tix
{
	TTexturePtr TTextureLoader::LoadTextureWithRegion(const TString& TextureName, int32 TargetMip, int32 StartX, int32 StartY, int32 EndX, int32 EndY)
	{
		TI_ASSERT(StartX % 4 == 0 && StartY % 4 == 0 && EndX % 4 == 0 && EndY % 4 == 0);
		TFile File;
		if (!File.Open(TextureName, EFA_READ))
		{
			TString FullPath = TPath::GetAbsolutePath(TextureName);
			if (!File.Open(FullPath, EFA_READ))
			{
				return nullptr;
			}
		}

		// Read header
		TResfileHeader ResHeader;
		File.Read(&ResHeader, sizeof(TResfileHeader), sizeof(TResfileHeader));
		if (ResHeader.Version != TIRES_VERSION_MAINFILE)
		{
			TI_ASSERT(0);
			return nullptr;
		}

		// Read chunk
		TResfileChunkHeader ChunkHeader;
		TI_ASSERT(ResHeader.ChunkCount == 1);
		File.Read(&ChunkHeader, sizeof(TResfileChunkHeader), sizeof(TResfileChunkHeader));
		TI_ASSERT(ChunkHeader.ID == TIRES_ID_CHUNK_TEXTURE);
		TI_ASSERT(ChunkHeader.Version == TIRES_VERSION_CHUNK_TEXTURE);

		const int32 TextureCount = ChunkHeader.ElementCount;
		TI_ASSERT(TextureCount == 1);

		THeaderTexture TextureHeader;
		File.Read(&TextureHeader, sizeof(THeaderTexture), sizeof(THeaderTexture));

		TTextureDesc Desc;
		Desc.Type = (E_TEXTURE_TYPE)TextureHeader.Type;
		Desc.Format = (E_PIXEL_FORMAT)TextureHeader.Format;
		Desc.Width = EndX - StartX;
		Desc.Height = EndY - StartY;
		Desc.AddressMode = (E_TEXTURE_ADDRESS_MODE)TextureHeader.AddressMode;
		Desc.SRGB = TextureHeader.SRGB;
		Desc.Mips = 1;
		if (Desc.SRGB != 0)
		{
			Desc.Format = GetSRGBFormat(Desc.Format);
		}
		TI_ASSERT(Desc.Type == ETT_TEXTURE_2D);

		TTexturePtr Texture = ti_new TTexture(Desc);
		int32 DataOffset = 0;
		int32 W = TextureHeader.Width;
		int32 H = TextureHeader.Height;
		for (uint32 m = 0; m < TextureHeader.Mips; ++m)
		{
			if (m == TargetMip)
			{
				int32 DstDataSize = TImage::GetDataSize(Desc.Format, Desc.Width, Desc.Height);
				uint8* DstData = ti_new uint8[DstDataSize];
				int32 SrcRowPitch = TImage::GetRowPitch(Desc.Format, W);
				vector2di BlockSize = TImage::GetBlockSize(Desc.Format);
				int32 BlockSizeInBytes = TImage::GetBlockSizeInBytes(Desc.Format);

				DataOffset += StartY / BlockSize.Y * SrcRowPitch + StartX / BlockSize.X * BlockSizeInBytes;
				File.Seek(DataOffset, true);

				int32 DstRowPitch = (Desc.Width + BlockSize.X - 1) / BlockSize.X * BlockSizeInBytes;
				int32 SrcRowOffset = SrcRowPitch - DstRowPitch;
				int32 DstDataOffset = 0;
				for (int32 y = StartY; y < EndY; y += BlockSize.Y)
				{
					File.Read(DstData + DstDataOffset, DstRowPitch, DstRowPitch);
					DstDataOffset += DstRowPitch;
					File.Seek(SrcRowOffset, true);
				}

				Texture->AddSurface(Desc.Width, Desc.Height, DstData, DstRowPitch, DstDataSize);
				ti_delete[] DstData;
				break;
			}
			else
			{
				int32 DataSize = TImage::GetDataSize(Desc.Format, W, H);
				DataOffset += sizeof(uint32) * 4 + DataSize;
				DataOffset = ti_align4(DataOffset);
			}
			W /= 2;
			H /= 2;
		}
		return Texture;
	}
}
