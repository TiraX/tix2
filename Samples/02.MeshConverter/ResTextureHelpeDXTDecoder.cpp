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

	void DecompressDXT1(TResTextureDefine* Texture, TVector<TImage*>& Images) 
	{
		uint32 BlockW = (Texture->Desc.Width + 3) >> 2;
		uint32 BlockH = (Texture->Desc.Height + 3) >> 2;

		const uint32 BlockSize = GetBlockSize(Texture->Desc.Format);
		TI_ASSERT(BlockSize != 0);
		if (BlockSize == 0)
		{
			printf("Error: Invalid block size.\n");
			return;
		}

		if (Texture->Desc.Type != ETT_TEXTURE_2D)
		{
			printf("Error: Not support DXT1 format other than Texture2D.");
			return;
		}

		// decode mipmaps
		int32 Width = Texture->Desc.Width;
		int32 Height = Texture->Desc.Height;

		for (int32 mip = 0; mip < (int32)Texture->Surfaces.size(); ++mip)
		{
			uint8 * InputData = reinterpret_cast<uint8 *>(Texture->Surfaces[mip].Data.GetBuffer());
			TImage* TGAImage = ti_new TImage(EPF_RGB8, Width, Height);

			SColor BlockColor[16];
			memset(BlockColor, 0xFF, sizeof(BlockColor));

			BlockW = (Width + 3) >> 2;
			BlockH = (Height + 3) >> 2;

			for (uint32 h = 0; h < BlockH; h++)
			{
				for (uint32 w = 0; w < BlockW; w++)
				{
					uint32 Offset = (h * BlockW + w) * BlockSize;
					DecompressDXT1Block(InputData + Offset, BlockColor, true);

					uint32 DecompWidth = ti_min(4, Width - w * 4);
					uint32 DecompHeight = ti_min(4, Height - h * 4);

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

			Width /= 2;
			Height /= 2;

			Images.push_back(TGAImage);
		}
	}

	void DecompressDXT5(TResTextureDefine* Texture, TVector<TImage*>& Images)
	{
		uint32 BlockW = (Texture->Desc.Width + 3) >> 2;
		uint32 BlockH = (Texture->Desc.Height + 3) >> 2;

		const uint32 BlockSize = GetBlockSize(Texture->Desc.Format);
		TI_ASSERT(BlockSize != 0);
		if (BlockSize == 0)
		{
			printf("Error: Invalid block size.\n");
			return;
		}

		if (Texture->Desc.Type != ETT_TEXTURE_2D)
		{
			printf("Error: Not support DXT5 format other than Texture2D.");
			return;
		}

		int32 Width = Texture->Desc.Width;
		int32 Height = Texture->Desc.Height;

		// Decode mipmaps
		for (int32 mip = 0; mip < (int32)Texture->Surfaces.size(); ++mip)
		{
			uint8 * InputData = reinterpret_cast<uint8 *>(Texture->Surfaces[mip].Data.GetBuffer());
			TImage* TGAImage = ti_new TImage(EPF_RGBA8, Width, Height);

			SColor BlockColor[16];
			memset(BlockColor, 0xFF, sizeof(BlockColor));

			BlockW = (Width + 3) >> 2;
			BlockH = (Height + 3) >> 2;

			for (uint32 h = 0; h < BlockH; h++)
			{
				for (uint32 w = 0; w < BlockW; w++)
				{
					uint32 Offset = (h * BlockW + w) * BlockSize;
					DecompressDXT5Block(InputData + Offset, BlockColor);
					DecompressDXT1Block(InputData + Offset + BlockSize / 2, BlockColor, false);

					uint32 decompWidth = ti_min(4, Width - w * 4);
					uint32 decompHeight = ti_min(4, Height - h * 4);

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

			Width /= 2;
			Height /= 2;

			Images.push_back(TGAImage);
		}
	}

	void DecodeDXT(TResTextureDefine* Texture, TVector<TImage*>& Images)
	{
		if (Texture == nullptr)
		{
			return;
		}

		if (Texture->Desc.Format == EPF_DDS_DXT1)
		{
			DecompressDXT1(Texture, Images);
		}
		else if (Texture->Desc.Format == EPF_DDS_DXT5)
		{
			DecompressDXT5(Texture, Images);
		}
		else if (Texture->Desc.Format == EPF_A8)
		{
			if (Texture->Desc.Type != ETT_TEXTURE_2D)
			{
				printf("Error: Not support EPF_A8 format other than Texture2D.");
				return;
			}
			// Copy pixel data to TImage directly
			int32 W = Texture->Desc.Width;
			int32 H = Texture->Desc.Height;
			for (uint32 mip = 0; mip < Texture->Desc.Mips; ++mip)
			{
				TResSurfaceData& MipData = Texture->Surfaces[mip];
				TImage * Image = ti_new TImage(EPF_A8, W, H);
				TI_ASSERT(W * H == MipData.Data.GetLength());
				memcpy(Image->Lock(), MipData.Data.GetBuffer(), W * H);
				Image->Unlock();

				W /= 2;
				H /= 2;
				Images.push_back(Image);
			}
		}
		else if (Texture->Desc.Format == EPF_RGBA8)
		{
			int32 W = Texture->Desc.Width;
			int32 H = Texture->Desc.Height;
			// make sure alpha have value
			bool HasAlpha = false;

			int32 Faces = 1;
			if (Texture->Desc.Type == ETT_TEXTURE_CUBE)
				Faces = 6;

			for (int32 f = 0; f < Faces; ++f)
			{
				uint8* Data = (uint8*)Texture->Surfaces[f * Texture->Desc.Mips + 0].Data.GetBuffer();
				for (int32 y = 0; y < H; ++y)
				{
					for (int32 x = 0; x < W; ++x)
					{
						uint8* c = Data + (y * W + x) * 4;
						if (c[3] > 0)
						{
							HasAlpha = true;
							break;
						}
					}
				}
			}
			for (int32 f = 0; f < Faces; ++f)
			{
				W = Texture->Desc.Width;
				H = Texture->Desc.Height;
				for (uint32 mip = 0; mip < Texture->Desc.Mips; ++mip)
				{
					TResSurfaceData& MipData = Texture->Surfaces[f * Texture->Desc.Mips + mip];
					uint8* Data = (uint8*)MipData.Data.GetBuffer();
					TImage * Image = ti_new TImage(HasAlpha ? EPF_RGBA8 : EPF_RGB8, W, H);
					TI_ASSERT(W * H * 4 == MipData.Data.GetLength());

					for (int32 y = 0; y < H; ++y)
					{
						for (int32 x = 0; x < W; ++x)
						{
							uint8* cd = Data + (y * W + x) * 4;
							SColor c;
							c.R = cd[2];
							c.G = cd[1];
							c.B = cd[0];
							c.A = cd[3];
							Image->SetPixel(x, y, c);
						}
					}

					W /= 2;
					H /= 2;
					Images.push_back(Image);
				}
			}
		}
		else if (Texture->Desc.Format == EPF_RGBA32F)
		{
			int32 W = Texture->Desc.Width;
			int32 H = Texture->Desc.Height;
			// make sure alpha have value
			bool HasAlpha = false;

			int32 Faces = 1;
			if (Texture->Desc.Type == ETT_TEXTURE_CUBE)
				Faces = 6;

			for (int32 f = 0; f < Faces; ++f)
			{
				float* Data = (float*)Texture->Surfaces[f * Texture->Desc.Mips + 0].Data.GetBuffer();
				for (int32 y = 0; y < H; ++y)
				{
					for (int32 x = 0; x < W; ++x)
					{
						float* c = Data + (y * W + x) * 4;
						if (c[3] < 1.f)
						{
							HasAlpha = true;
							break;
						}
					}
				}
			}

			for (int32 f = 0; f < Faces; ++f)
			{
				W = Texture->Desc.Width;
				H = Texture->Desc.Height;
				for (uint32 mip = 0; mip < Texture->Desc.Mips; ++mip)
				{
					TResSurfaceData& MipData = Texture->Surfaces[f * Texture->Desc.Mips + mip];
					float* Data = (float*)MipData.Data.GetBuffer();
					TImage * Image = ti_new TImage(HasAlpha ? EPF_RGBA16F : EPF_RGB16F, W, H);
					TI_ASSERT(W * H * sizeof(float) * 4 == MipData.Data.GetLength());

					for (int32 y = 0; y < H; ++y)
					{
						for (int32 x = 0; x < W; ++x)
						{
							float* cd = Data + (y * W + x) * 4;
							SColorf c;
							c.R = cd[2];
							c.G = cd[1];
							c.B = cd[0];
							c.A = cd[3];
							Image->SetPixel(x, y, c);
						}
					}

					W /= 2;
					H /= 2;
					Images.push_back(Image);
				}
			}
		}
		else
		{
			TI_ASSERT(0);
			printf("Error: Unsupported dds format.\n");
		}

		if (Images.size() > 0)
		{
			for (int32 i = 0; i < (int32)Images.size(); ++i)
			{
				TImage* TGAImage = Images[i];
				TGAImage->FlipY();
			}
		}
	}
}
