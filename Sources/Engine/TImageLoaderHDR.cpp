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

	TImagePtr TImage::LoadImageHDR(TFile& FileInput, bool UseHalf)
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
		E_PIXEL_FORMAT TargetFormat = UseHalf ? EPF_RGBA16F : EPF_RGBA32F;
		TImagePtr HdrImage = ti_new TImage(TargetFormat, W, H);
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

	static SColor ToRGBEDithered(const SColorf& ColorIN)
	{
		const float R = ColorIN.R;
		const float G = ColorIN.G;
		const float B = ColorIN.B;
		const float Primary = TMath::Max3(R, G, B);
		SColor	ReturnColor;

		if (Primary < 1E-32)
		{
			ReturnColor = SColor(0, 0, 0, 0);
		}
		else
		{
			int32 Exponent;
			const float Scale = frexp(Primary, &Exponent) / Primary * 255.f;

			ReturnColor.R = TMath::Clamp(int32((R * Scale) + TMath::RandomUnit()), 0, 255);
			ReturnColor.G = TMath::Clamp(int32((G * Scale) + TMath::RandomUnit()), 0, 255);
			ReturnColor.B = TMath::Clamp(int32((B * Scale) + TMath::RandomUnit()), 0, 255);
			ReturnColor.A = TMath::Clamp(int32(Exponent), -128, 127) + 128;
		}

		return ReturnColor;
	}

	// From UE4, ImageUtils.cpp
	void WriteHDRHeader(TFile& File, TImage* Image)
	{
		const int32 MaxHeaderSize = 256;
		char Header[MaxHeaderSize];
		sprintf(Header, "#?RADIANCE\nFORMAT=32-bit_rle_rgbe\n\n-Y %d +X %d\n", Image->GetHeight(), Image->GetWidth());
		Header[MaxHeaderSize - 1] = 0;
		int32 Len = TMath::Min((int32)strlen(Header), MaxHeaderSize);
		File.Write(Header, Len);
	}

	void WriteScanLine(TFile& File, const TVector<uint8>& ScanLine)
	{
		const uint8* LineEnd = ScanLine.data() + ScanLine.size();
		const uint8* LineSource = ScanLine.data();
		TVector<uint8> Output;
		Output.reserve(ScanLine.size() * 2);
		while (LineSource < LineEnd)
		{
			int32 CurrentPos = 0;
			int32 NextPos = 0;
			int32 CurrentRunLength = 0;
			while (CurrentRunLength <= 4 && NextPos < 128 && LineSource + NextPos < LineEnd)
			{
				CurrentPos = NextPos;
				CurrentRunLength = 0;
				while (CurrentRunLength < 127 && CurrentPos + CurrentRunLength < 128 && LineSource + NextPos < LineEnd && LineSource[CurrentPos] == LineSource[NextPos])
				{
					NextPos++;
					CurrentRunLength++;
				}
			}

			if (CurrentRunLength > 4)
			{
				// write a non run: LineSource[0] to LineSource[CurrentPos]
				if (CurrentPos > 0)
				{
					Output.push_back(CurrentPos);
					for (int32 i = 0; i < CurrentPos; i++)
					{
						Output.push_back(LineSource[i]);
					}
				}
				Output.push_back((uint8)(128 + CurrentRunLength));
				Output.push_back(LineSource[CurrentPos]);
			}
			else
			{
				// write a non run: LineSource[0] to LineSource[NextPos]
				Output.push_back((uint8)(NextPos));
				for (int32 i = 0; i < NextPos; i++)
				{
					Output.push_back((uint8)(LineSource[i]));
				}
			}
			LineSource += NextPos;
		}
		File.Write(Output.data(), (int32)Output.size());
	}

	void WriteHDRBits(TFile& File, TImage* HdrImage)
	{
		//const FRandomStream RandomStream(0xA1A1);
		const int32 NumChannels = 4;
		const int32 SizeX = HdrImage->GetWidth();
		const int32 SizeY = HdrImage->GetHeight();
		TVector<uint8> ScanLine[NumChannels];
		for (int32 Channel = 0; Channel < NumChannels; Channel++)
		{
			ScanLine[Channel].reserve(SizeX);
		}

		for (int32 y = 0; y < SizeY; y++)
		{
			// write RLE header
			uint8 RLEheader[4];
			RLEheader[0] = 2;
			RLEheader[1] = 2;
			RLEheader[2] = SizeX >> 8;
			RLEheader[3] = SizeX & 0xFF;
			File.Write(&RLEheader[0], sizeof(RLEheader));

			for (int32 Channel = 0; Channel < NumChannels; Channel++)
			{
				ScanLine[Channel].clear();
			}

			for (int32 x = 0; x < SizeX; x++)
			{
				SColorf HdrColor = HdrImage->GetPixelFloat(x, y);
				SColor RGBEColor = ToRGBEDithered(HdrColor);

				ScanLine[0].push_back(RGBEColor.R);
				ScanLine[1].push_back(RGBEColor.G);
				ScanLine[2].push_back(RGBEColor.B);
				ScanLine[3].push_back(RGBEColor.A);
			}

			for (int32 Channel = 0; Channel < NumChannels; Channel++)
			{
				WriteScanLine(File, ScanLine[Channel]);
			}
		}
	}

	bool TImage::SaveToHDR(const char* filename, int32 MipIndex)
	{
		TFile HdrFile;
		if (!HdrFile.Open(filename, EFA_CREATEWRITE))
		{
			return false;
		}

		WriteHDRHeader(HdrFile, this);
		WriteHDRBits(HdrFile, this);
		HdrFile.Close();

		return true;
	}
}
