/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "ResHelper.h"
#include "ResTextureHelper.h"
#include "dds.h"
#include "TImage.h"

namespace tix
{
	inline static uint32 GetBlockSize(E_PIXEL_FORMAT Format) 
	{
		switch (Format)
		{
		case EPF_DDS_DXT1:
			return 8;
		case EPF_DDS_DXT5:
			return 16;
        default:
            return 0;
		}
    }

	inline SColor GetColor565(uint16 ColorValue)
	{
		const float inv_5 = 1.f / 0x1f * 255.f;
		const float inv_6 = 1.f / 0x3f * 255.f;

		SColor Result;
		Result.B = (uint8)((ColorValue & 0x1f) * inv_5);
		Result.G = (uint8)(((ColorValue & 0x7e0) >> 5) * inv_6);
		Result.R = (uint8)(((ColorValue & 0xf800) >> 11) * inv_5);
		return Result;
	}
	
	void DecompressDXT1Block(const uint8 *Block, SColor * OutColors, bool bCheckOrder) 
	{
		uint16 ColorA = (Block[1] << 8) | Block[0];
		uint16 ColorB = (Block[3] << 8) | Block[2];

		uint32 Mod = reinterpret_cast<const uint32 *>(Block + 4)[0];

		SColor Colors[4];
		SColorf Af, Bf;
		Colors[0] = GetColor565(ColorA);
		Colors[1] = GetColor565(ColorB);

		Af = Colors[0];
		Bf = Colors[1];

		const float inv_3 = 1.f / 3.f;
		const uint16 *BlockPtr = reinterpret_cast<const uint16 *>(Block);
		if (!bCheckOrder || BlockPtr[0] > BlockPtr[1])
		{
			Colors[2] = (Af * 2.f + Bf) * inv_3;
			Colors[3] = (Af + Bf * 2.f) * inv_3;
		}
		else 
		{
			Colors[2] = (Af + Bf) * 0.5f;
		}
		
		for (uint32 i = 0; i < 16; i++) 
		{
			const SColor& c = Colors[(Mod >> (i * 2)) & 3];
			OutColors[i].R = c.R;
			OutColors[i].G = c.G;
			OutColors[i].B = c.B;
		}
	}

	void DecompressDXT5Block(const uint8 *Block, SColor * OutColors)
	{
		int32 Alpha0 = Block[0];
		int32 Alpha1 = Block[1];

		int32 Palette[8];
		Palette[0] = Alpha0;
		Palette[1] = Alpha1;

		if (Alpha0 > Alpha1) 
		{
			for (int32 i = 2; i < 8; ++i) 
			{
				Palette[i] = ((8 - i) * Alpha0 + (i - 1) * Alpha1) / 7;
			}
		}
		else
		{
			for (int32 i = 2; i < 6; ++i) 
			{
				Palette[i] = ((6 - i) * Alpha0 + (i - 1) * Alpha1) / 5;
			}
			Palette[6] = 0;
			Palette[7] = 255;
		}

		uint64 Mod = *reinterpret_cast<const uint64 *>(Block) >> 16;
		for (uint32 i = 0; i < 16; i++) 
		{
			OutColors[i].A = Palette[(Mod >> (i * 3)) & 7] & 0xff;
		}
	}

	TImage* DecompressDXT1(TResTextureDefine* Texture) 
	{
		uint32 BlockW = (Texture->Desc.Width + 3) >> 2;
		uint32 BlockH = (Texture->Desc.Height + 3) >> 2;

		const uint32 BlockSize = GetBlockSize(Texture->Desc.Format);
		TI_ASSERT(BlockSize != 0);
		if (BlockSize == 0)
		{
			printf("Error: Invalid block size.\n");
			return nullptr;
		}

		// Only decode mipmap 0
		uint8 * InputData = reinterpret_cast<uint8 *>(Texture->Surfaces[0].Data.GetBuffer());
		TImage* TGAImage = ti_new TImage(EPF_RGB8, Texture->Desc.Width, Texture->Desc.Height);
		
		SColor BlockColor[16];
		memset(BlockColor, 0xFF, sizeof(BlockColor));

		for (uint32 h = 0; h < BlockH; h++) 
		{
			for (uint32 w = 0; w < BlockW; w++) 
			{
				uint32 Offset = (h * BlockW + w) * BlockSize;
				DecompressDXT1Block(InputData + Offset, BlockColor, true);

				uint32 DecompWidth = ti_min(4, Texture->Desc.Width - w * 4);
				uint32 DecompHeight = ti_min(4, Texture->Desc.Height - h * 4);

				for (uint32 y = 0; y < DecompHeight; y++)
				{
					for (uint32 x = 0; x < DecompWidth; x++)
					{
						int32 StartX = w * 4;
						int32 StartY = h * 4;
						TGAImage->SetPixel(StartX + x, StartY + y, BlockColor[y * 4 + x]);
					}
				}
			}
		}
		return TGAImage;
	}

	TImage* DecompressDXT5(TResTextureDefine* Texture)
	{
		uint32 BlockW = (Texture->Desc.Width + 3) >> 2;
		uint32 BlockH = (Texture->Desc.Height + 3) >> 2;

		const uint32 BlockSize = GetBlockSize(Texture->Desc.Format);
		TI_ASSERT(BlockSize != 0);
		if (BlockSize == 0)
		{
			printf("Error: Invalid block size.\n");
			return nullptr;
		}

		// Only decode mipmap 0
		uint8 * InputData = reinterpret_cast<uint8 *>(Texture->Surfaces[0].Data.GetBuffer());
		TImage* TGAImage = ti_new TImage(EPF_RGBA8, Texture->Desc.Width, Texture->Desc.Height);

		SColor BlockColor[16];
		memset(BlockColor, 0xFF, sizeof(BlockColor));

		for (uint32 h = 0; h < BlockH; h++) 
		{
			for (uint32 w = 0; w < BlockW; w++) 
			{
				uint32 Offset = (h * BlockW + w) * BlockSize;
				DecompressDXT5Block(InputData + Offset, BlockColor);
				DecompressDXT1Block(InputData + Offset + BlockSize / 2, BlockColor, false);

				uint32 decompWidth = ti_min(4, Texture->Desc.Width - w * 4);
				uint32 decompHeight = ti_min(4, Texture->Desc.Height - h * 4);

				for (uint32 y = 0; y < decompHeight; y++)
				{
					for (uint32 x = 0; x < decompWidth; x++) 
					{
						int32 StartX = w * 4;
						int32 StartY = h * 4;
						TGAImage->SetPixel(StartX + x, StartY + y, BlockColor[y * 4 + x]);
					}
				}
			}
		}
		return TGAImage;
	}

	bool DecodeDXT(const TString& SrcName, const TString& DstName)
	{
		TResTextureDefine* Texture = TResTextureHelper::LoadDdsFile(SrcName);

		if (Texture == nullptr)
		{
			return false;
		}

		TImage* TGAImage = nullptr;
		if (Texture->Desc.Format == EPF_DDS_DXT1)
		{
			TGAImage = DecompressDXT1(Texture);
		}
		else if (Texture->Desc.Format == EPF_DDS_DXT5)
		{
			TGAImage = DecompressDXT5(Texture);
		}
		else
		{
			TI_ASSERT(0);
			printf("Error: Unsupported dds format.\n");
		}

		if (TGAImage != nullptr)
		{
			TGAImage->FlipY();
			TGAImage->SaveToTga(DstName.c_str());
			ti_delete TGAImage;
		}

		return true;
	}
}
