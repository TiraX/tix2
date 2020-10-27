/*
	Copyright (C) 2012
	By Zhao Shuai tirax.cn@gmail.com 2012.9.04
*/

#include "stdafx.h"
#include "TImage.h"

namespace tix
{
	void GetHeaderLine(const uint8*& BufferPos, int8 Line[256])
	{
		int8* LinePtr = Line;

		uint32 i;

		for (i = 0; i < 255; ++i)
		{
			if (*BufferPos == 0 || *BufferPos == 10 || *BufferPos == 13)
			{
				++BufferPos;
				break;
			}

			*LinePtr++ = *BufferPos++;
		}

		Line[i] = 0;
	}

	TImagePtr TImage::LoadImageHDR(TFile& FileInput)
	{
		FileInput.Seek(0);
		uint8* ImageData = ti_new uint8[FileInput.GetSize()];
		FileInput.Read(ImageData, FileInput.GetSize(), FileInput.GetSize());
		FileInput.Close();

		int8 Line[256];
		const uint8* DataPtr = ImageData;
		GetHeaderLine(DataPtr, Line);
		if (strcmp(Line, "#?RADIANCE") != 0)
		{
			_LOG(Error, "Invalid HDR sig. [%s]\n", FileInput.GetFileName().c_str());
			ti_delete[] ImageData;
			return nullptr;
		}

		const uint8* RGBDataStart = nullptr;
		int32 W = 0, H = 0;
		for (;;)
		{
			GetHeaderLine(DataPtr, Line);

			int8* HeightStr = strstr(Line, "-Y ");
			int8* WidthStr = strstr(Line, "+X ");

			if (HeightStr != nullptr && WidthStr != nullptr)
			{
				// insert a /0 after the height value
				*(WidthStr - 1) = 0;

				H = atoi(HeightStr + 3);
				W = atoi(WidthStr + 3);

				RGBDataStart = DataPtr;
				break;
			}
		}

		// minimum and maxmimum scanline length for encoding
		uint32 MINELEN = 8;
		uint32 MAXELEN = 0x7fff;
		uint32 W1 = (uint32)W;
		if (RGBDataStart == nullptr || W == 0 || H == 0 || W1 < MINELEN || W1 > MAXELEN)
		{
			// Invalid hdr image
			_LOG(Error, "Invalid HDR data. [%s]\n", FileInput.GetFileName().c_str());
			ti_delete[] ImageData;
			return nullptr;
		}

		// Decompress Image
		TImagePtr HdrImage = ti_new TImage(EPF_RGBA16F, W, H);
		const uint8* LineDataPtr = RGBDataStart;
		SColor * RGBELine = ti_new SColor[W];

		for (int32 y = 0; y < H; ++y)
		{
			uint32 Len = W;

			uint8 r = *LineDataPtr++;
			uint8 g = *LineDataPtr++;
			uint8 b = *LineDataPtr++;
			uint8 e = *LineDataPtr++;

			if (r != 2 || g != 2 || (b & 128))
			{
				// invalid , use OldDecompressScanline() in UE4 to Decompress image
				HdrImage = nullptr;
				break;
			}
			for (uint32 Channel = 0; Channel < 4; ++Channel)
			{
				SColor* DestRGBE = RGBELine;

				for (uint32 MultiRunIndex = 0; MultiRunIndex < Len;)
				{
					uint8 c = *LineDataPtr++;

					if (c > 128)
					{
						uint32 Count = c & 0x7f;

						c = *LineDataPtr++;

						for (uint32 RunIndex = 0; RunIndex < Count; ++RunIndex)
						{
							(*DestRGBE)[Channel] = c;
							DestRGBE++;
						}
						MultiRunIndex += Count;
					}
					else
					{
						uint32 Count = c;

						for (uint32 RunIndex = 0; RunIndex < Count; ++RunIndex)
						{
							(*DestRGBE)[Channel] = *LineDataPtr++;
							DestRGBE++;
						}
						MultiRunIndex += Count;
					}
				}
			}

			// Convert RGBE to half
			for (uint32 x = 0; x < Len; x++)
			{
				float scale = ldexp(1.f, RGBELine[x].A - (128 + 8));
				SColorf Color;
				Color.R = RGBELine[x].R * scale;
				Color.G = RGBELine[x].G * scale;
				Color.B = RGBELine[x].B * scale;
				Color.A = 1.f;
				HdrImage->SetPixel(x, y, Color);
			}
		}

		ti_delete[] RGBELine;
		ti_delete[] ImageData;
		return HdrImage;
	}
}
