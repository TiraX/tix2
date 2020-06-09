/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TVTTextureLoader.h"

namespace tix
{
	TVTTextureLoader::TVTTextureLoader(const TString& InFileName)
	{
		if (!TextureFile.Open(InFileName, EFA_READ))
		{
			TString FullPath = TPath::GetAbsolutePath(InFileName);
			if (!TextureFile.Open(FullPath, EFA_READ))
			{
				_LOG(Error, "Failed to open file %s\n", InFileName.c_str());
				TI_ASSERT(0);
			}
		}

		// Read header
		TResfileHeader ResHeader;
		TextureFile.Read(&ResHeader, sizeof(TResfileHeader), sizeof(TResfileHeader));
		if (ResHeader.Version != TIRES_VERSION_MAINFILE)
		{
			_LOG(Error, "Invalid version of %s\n", InFileName.c_str());
			TI_ASSERT(0);
		}

		// Read chunk
		TResfileChunkHeader ChunkHeader;
		TI_ASSERT(ResHeader.ChunkCount == 1);
		TextureFile.Read(&ChunkHeader, sizeof(TResfileChunkHeader), sizeof(TResfileChunkHeader));
		TI_ASSERT(ChunkHeader.ID == TIRES_ID_CHUNK_TEXTURE);
		TI_ASSERT(ChunkHeader.Version == TIRES_VERSION_CHUNK_TEXTURE);

		const int32 TextureCount = ChunkHeader.ElementCount;
		TI_ASSERT(TextureCount == 1);

		THeaderTexture TextureHeader;
		TextureFile.Read(&TextureHeader, sizeof(THeaderTexture), sizeof(THeaderTexture));

		TextureDesc.Type = (E_TEXTURE_TYPE)TextureHeader.Type;
		TextureDesc.Format = (E_PIXEL_FORMAT)TextureHeader.Format;
		TextureDesc.Width = TextureHeader.Width;
		TextureDesc.Height = TextureHeader.Height;
		TextureDesc.AddressMode = (E_TEXTURE_ADDRESS_MODE)TextureHeader.AddressMode;
		TextureDesc.SRGB = TextureHeader.SRGB;
		TextureDesc.Mips = TextureHeader.Mips;
		if (TextureDesc.SRGB != 0)
		{
			TextureDesc.Format = GetSRGBFormat(TextureDesc.Format);
		}
		TI_ASSERT(TextureDesc.Type == ETT_TEXTURE_2D);

		DataOffset = TextureFile.Tell();
	}

	TTexturePtr TVTTextureLoader::LoadTextureWithRegion(int32 TargetMip, int32 StartX, int32 StartY, int32 EndX, int32 EndY)
	{
		TI_ASSERT(StartX % 4 == 0 && StartY % 4 == 0 && EndX % 4 == 0 && EndY % 4 == 0);
		TextureFile.Seek(DataOffset, false);

		TTextureDesc PageDesc = TextureDesc;
		PageDesc.Width = EndX - StartX;
		PageDesc.Height = EndY - StartY;
		PageDesc.Mips = 1;

		TTexturePtr Texture = ti_new TTexture(PageDesc);
		int32 DataOffset = 0;
		int32 W = TextureDesc.Width;
		int32 H = TextureDesc.Height;
		for (uint32 m = 0; m < TextureDesc.Mips; ++m)
		{
			if (m == TargetMip)
			{
				int32 DstDataSize = TImage::GetDataSize(PageDesc.Format, PageDesc.Width, PageDesc.Height);
				uint8* DstData = ti_new uint8[DstDataSize];
				int32 SrcRowPitch = TImage::GetRowPitch(PageDesc.Format, W);
				vector2di BlockSize = TImage::GetBlockSize(PageDesc.Format);
				int32 BlockSizeInBytes = TImage::GetBlockSizeInBytes(PageDesc.Format);

				DataOffset += StartY / BlockSize.Y * SrcRowPitch + StartX / BlockSize.X * BlockSizeInBytes;
				TextureFile.Seek(DataOffset, true);

				int32 DstRowPitch = (PageDesc.Width + BlockSize.X - 1) / BlockSize.X * BlockSizeInBytes;
				int32 SrcRowOffset = SrcRowPitch - DstRowPitch;
				int32 DstDataOffset = 0;
				for (int32 y = StartY; y < EndY; y += BlockSize.Y)
				{
					TextureFile.Read(DstData + DstDataOffset, DstRowPitch, DstRowPitch);
					DstDataOffset += DstRowPitch;
					TextureFile.Seek(SrcRowOffset, true);
				}

				Texture->AddSurface(PageDesc.Width, PageDesc.Height, DstData, DstRowPitch, DstDataSize);
				ti_delete[] DstData;
				break;
			}
			else
			{
				int32 DataSize = TImage::GetDataSize(PageDesc.Format, W, H);
				DataOffset += sizeof(uint32) * 4 + DataSize;
				DataOffset = ti_align4(DataOffset);
			}
			W /= 2;
			H /= 2;
		}
		return Texture;
	}

	TTexturePtr TVTTextureLoader::LoadBakedVTPages(int32 Mip, int32 PageX, int32 PageY)
	{
		TTextureDesc TextureDesc;
		TextureDesc.Type = ETT_TEXTURE_2D;
		TextureDesc.Format = FVTSystem::PageFormat;
		TextureDesc.Width = FVTSystem::PPSize;
		TextureDesc.Height = FVTSystem::PPSize;
		TextureDesc.AddressMode = ETC_REPEAT;
		TextureDesc.SRGB = 1;
		TextureDesc.Mips = 1;

		int8 TextureName[128];
		sprintf(TextureName, "vt_pages/%02d_%02d_%02d.page", Mip, PageX, PageY);
		TFile F;
		if (F.Open(TextureName, EFA_READ))
		{
			TTexturePtr Texture = ti_new TTexture(TextureDesc);
			
			int32 Size = TImage::GetDataSize(FVTSystem::PageFormat, FVTSystem::PPSize, FVTSystem::PPSize);
			int32 RowPitch = TImage::GetRowPitch(FVTSystem::PageFormat, FVTSystem::PPSize);
			TI_ASSERT(Size == F.GetSize());

			uint8* Buffer = ti_new uint8[Size];
			F.Read(Buffer, Size, Size);
			Texture->AddSurface(FVTSystem::PPSize, FVTSystem::PPSize, Buffer, RowPitch, Size);
			ti_delete[] Buffer;

			return Texture;
		}
		else
		{
			_LOG(Error, "Failed to load Page : %s\n", TextureName);
			TI_ASSERT(0);
			return nullptr;
		}
	}
}
