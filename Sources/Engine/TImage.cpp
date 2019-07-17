/*
	Copyright (C) 2012
	By Zhao Shuai tirax.cn@gmail.com 2012.9.04
*/

#include "stdafx.h"
#include "TImage.h"

namespace tix
{
	int32 TImage::GetPixelSizeInBytes(E_PIXEL_FORMAT Format)
	{
		TI_ASSERT(!IsCompressedFormat(Format));
		switch (Format)
		{
		case EPF_A8:
			return 1;
		case EPF_RGB8:
		case EPF_BGR8:
			return 3;
		case EPF_RGBA8:
		case EPF_RGBA8_SRGB:
		case EPF_BGRA8:
		case EPF_BGRA8_SRGB:
			return 4;

		case EPF_R16F:
			return 2;
		case EPF_RG16F:
			return 4;
		case EPF_RGBA16F:
			return 8;
		case EPF_R32F:
			return 4;
		case EPF_RG32F:
			return 8;
		case EPF_RGB32F:
			return 12;
		case EPF_RGBA32F:
			return 16;

				// Depth formats
		case EPF_DEPTH16:
			return 2;
		case EPF_DEPTH32:
		case EPF_DEPTH24_STENCIL8:
			return 4;
		case EPF_STENCIL8:
			return 1;
		default:
			return 0;
		}
	}

	int32 TImage::GetBlockSizeInBytes(E_PIXEL_FORMAT Format)
	{
		TI_ASSERT(IsCompressedFormat(Format));
		switch (Format)
		{
		case EPF_DDS_DXT1:
		case EPF_DDS_DXT1_SRGB:
		case EPF_DDS_DXT3:
		case EPF_DDS_DXT3_SRGB:
			return 8;
		case EPF_DDS_DXT5:
		case EPF_DDS_DXT5_SRGB:
		case EPF_DDS_BC5:
			return 16;

				// ASTC formats 
		case EPF_ASTC4x4:
		case EPF_ASTC4x4_SRGB:
		case EPF_ASTC6x6:
		case EPF_ASTC6x6_SRGB:
		case EPF_ASTC8x8:
		case EPF_ASTC8x8_SRGB:
			return 16;
		default:
			return 0;
		}
	}

	vector2di TImage::GetBlockSize(E_PIXEL_FORMAT Format)
	{
		TI_ASSERT(IsCompressedFormat(Format));
		switch (Format)
		{
		case EPF_DDS_DXT1:
		case EPF_DDS_DXT1_SRGB:
		case EPF_DDS_DXT3:
		case EPF_DDS_DXT3_SRGB:
		case EPF_DDS_DXT5:
		case EPF_DDS_DXT5_SRGB:
		case EPF_DDS_BC5:
			return vector2di(4, 4);

			// ASTC formats 
		case EPF_ASTC4x4:
		case EPF_ASTC4x4_SRGB:
			return vector2di(4, 4);
		case EPF_ASTC6x6:
		case EPF_ASTC6x6_SRGB:
			return vector2di(6, 6);
		case EPF_ASTC8x8:
		case EPF_ASTC8x8_SRGB:
			return vector2di(8, 8);
		default:
			return vector2di();
		}
	}

	bool TImage::IsCompressedFormat(E_PIXEL_FORMAT Format)
	{
		return Format >= EPF_DDS_DXT1;
	}

	inline int32 GetBlockWidth(int32 PixelWidth, int32 BlockSize)
	{
		return (PixelWidth + BlockSize - 1) / BlockSize;
	}

	int32 TImage::GetDataSize(E_PIXEL_FORMAT Format, int32 Width, int32 Height)
	{
		if (IsCompressedFormat(Format))
		{
			int32 BlockSize = GetBlockSizeInBytes(Format);
			TI_ASSERT(BlockSize != 0);
			vector2di Block = GetBlockSize(Format);
			int32 BlockW = GetBlockWidth(Width, Block.X);
			int32 BlockH = GetBlockWidth(Height, Block.Y);
			return (BlockSize * BlockW * BlockH);
		}
		else
		{
			int32 RowPitch = Width * GetPixelSizeInBytes(Format);
			TI_ASSERT(RowPitch != 0);
			return (RowPitch * Height);
		}
	}

	int32 TImage::GetRowPitch(E_PIXEL_FORMAT Format, int32 Width)
	{
		if (IsCompressedFormat(Format))
		{
			int32 BlockSize = GetBlockSizeInBytes(Format);
			TI_ASSERT(BlockSize != 0);
			vector2di Block = GetBlockSize(Format);
			int32 BlockW = GetBlockWidth(Width, Block.X);
			return (BlockSize * BlockW);
		}
		else
		{
			return Width * GetPixelSizeInBytes(Format);
		}
	}

	TImage::TImage(E_PIXEL_FORMAT InPixelFormat, int32 Width, int32 Height)
		: PixelFormat(InPixelFormat)
	{
		Mipmaps.push_back(TSurfaceData());
		TSurfaceData& Mip0 = Mipmaps[0];
		Mip0.W = Width;
		Mip0.H = Height;
		if (IsCompressedFormat(InPixelFormat))
		{
			vector2di Block = GetBlockSize(InPixelFormat);
			Mip0.BlockSize = GetBlockSizeInBytes(InPixelFormat);
			int32 BlockW = GetBlockWidth(Width, Block.X);
			Mip0.RowPitch = Mip0.BlockSize * BlockW;
		}
		else
		{
			Mip0.RowPitch = Width * GetPixelSizeInBytes(InPixelFormat);
		}
		Mip0.Data.ReserveAndFill(GetDataSize(InPixelFormat, Width, Height));
	}

	TImage::~TImage()
	{
		ClearMipmaps();
	}

	uint8* TImage::Lock(int32 MipIndex)
	{
		return (uint8*)Mipmaps[MipIndex].Data.GetBuffer();
	}

	void TImage::Unlock()
	{
	}

	void TImage::SetPixel(int32 x, int32 y, const SColor& c, int32 MipIndex)
	{
		int32 Pitch = Mipmaps[MipIndex].RowPitch;
		uint8* Data = (uint8*)Mipmaps[MipIndex].Data.GetBuffer();

		int32 offset = y * Pitch + x * GetPixelSizeInBytes(PixelFormat);
		switch (PixelFormat)
		{
		case EPF_RGBA8_SRGB:
		case EPF_RGBA8:
			Data[offset + 0] = c.R;
			Data[offset + 1] = c.G;
			Data[offset + 2] = c.B;
			Data[offset + 3] = c.A;
			break;
		case EPF_RGB8:
			Data[offset + 0] = c.R;
			Data[offset + 1] = c.G;
			Data[offset + 2] = c.B;
			break;
		default:
			TI_ASSERT(0);
			break;
		}
	}

	void TImage::SetPixel(int32 x, int32 y, const SColorf& c, int32 MipIndex)
	{
		int32 Pitch = Mipmaps[MipIndex].RowPitch;
		uint8* Data = (uint8*)Mipmaps[MipIndex].Data.GetBuffer();

		int32 offset = y * Pitch + x * GetPixelSizeInBytes(PixelFormat);
		float* fdata = (float*)(Data + offset);
		half* hdata = (half*)(Data + offset);
		switch (PixelFormat)
		{
		case EPF_RG32F:
			fdata[0] = c.R;
			fdata[1] = c.G;
			break;
		case EPF_RGBA32F:
			fdata[0] = c.R;
			fdata[1] = c.G;
			fdata[2] = c.B;
			fdata[3] = c.A;
			break;
		case EPF_R16F:
			hdata[0] = half(c.R);
			break;
		case EPF_RG16F:
			hdata[0] = half(c.R);
			hdata[1] = half(c.G);
			break;
		case EPF_RGBA16F:
			hdata[0] = half(c.R);
			hdata[1] = half(c.G);
			hdata[2] = half(c.B);
			hdata[3] = half(c.A);
			break;
		default:
			TI_ASSERT(0);
			break;
		}
	}

	void TImage::SetPixel(int32 x, int32 y, uint8 c, int32 MipIndex)
	{
		int32 Pitch = Mipmaps[MipIndex].RowPitch;
		uint8* Data = (uint8*)Mipmaps[MipIndex].Data.GetBuffer();

		int32 offset = y * Pitch + x * GetPixelSizeInBytes(PixelFormat);
		switch (PixelFormat)
		{
		case EPF_A8:
			Data[offset] = c;
			break;
		default:
			TI_ASSERT(0);
			break;
		}
	}

	SColor TImage::GetPixel(int32 x, int32 y, int32 MipIndex)
	{
		int32 Width = Mipmaps[MipIndex].W;
		int32 Height = Mipmaps[MipIndex].H;
		int32 Pitch = Mipmaps[MipIndex].RowPitch;
		uint8* Data = (uint8*)Mipmaps[MipIndex].Data.GetBuffer();

		if ( x >= Width) x = Width - 1;
		if ( y >= Height) y = Height - 1;
		if ( x < 0) x = 0;
		if ( y < 0) y = 0;

		SColor color;
		int32 offset = y * Pitch + x * GetPixelSizeInBytes(PixelFormat);
		switch (PixelFormat)
		{
		case EPF_RGBA8_SRGB:
		case EPF_RGBA8:
			{
				color.R = Data[offset + 0];
				color.G = Data[offset + 1];
				color.B = Data[offset + 2];
				color.A = Data[offset + 3];
			}
			break;
		case EPF_RGB8:
			{
				color.R = Data[offset + 0];
				color.G = Data[offset + 1];
				color.B = Data[offset + 2];
				color.A = 255;
			}
			break;
		case EPF_A8:
			{
				color.R = Data[offset];
				color.G = Data[offset];
				color.B = Data[offset];
				color.A = Data[offset];
			}
			break;
		default:
			TI_ASSERT(0);
		}
		return color;
	}

	SColor TImage::GetPixel(float x, float y, int32 MipIndex)
	{
		int32 xi=(int32)(x); if (x<0) xi--;   //these replace (incredibly slow) floor (Visual c++ 2003, AMD Athlon)
		int32 yi=(int32)(y); if (y<0) yi--;

		float t1=x-xi, t2=y-yi;
		float d=t1*t2;
		float b=t1-d;
		float c=t2-d;
		float a=1-t1-c;
		SColor rgb11,rgb21,rgb12,rgb22;
		rgb11 = GetPixel(xi, yi, MipIndex);
		rgb21 = GetPixel(xi+1, yi, MipIndex);
		rgb12 = GetPixel(xi, yi+1, MipIndex);
		rgb22 = GetPixel(xi+1, yi+1, MipIndex);

		SColor color;
		//calculate linear interpolation
		color.R	= (uint8) (a * rgb11.R + b * rgb21.R + c * rgb12.R + d * rgb22.R);
		color.G = (uint8) (a * rgb11.G + b * rgb21.G + c * rgb12.G + d * rgb22.G);
		color.B = (uint8) (a * rgb11.B + b * rgb21.B + c * rgb12.B + d * rgb22.B);
		color.A = (uint8) (a * rgb11.A + b * rgb21.A + c * rgb12.A + d * rgb22.A);

		return color;
	}

	inline float Cubic(float x)
	{
		// from http://entropymine.com/imageworsener/bicubic/
		// B-spline (blurry)
		//const float B = 1.0f;
		//const float C = 0.f;

		// Mitchell
		//const float B = 1.f / 3.f;
		//const float C = 1.f / 3.f;

		// Catmull-Rom
		//const float B = 0.f;
		//const float C = 1.f / 2.f;

		// Custom 1  (match photoshop the best)
		const float B = 0.f;
		const float C = 0.75f;

		// Custom 2
		//const float B = 0.f;
		//const float C = 1.f;
		
		
		float f = ti_abs(x);

		if (f >= 0.f &&  f <= 1.f)
		{
			return (2.f - 1.5f * B - C) * (f * f * f) + (-3.f + 2.f * B + C) * (f * f) + (1.f - 1.f / 3.f * B);
		}
		else if (f > 1.f && f <= 2.f)
		{
			return (-1.f / 6.f * B - C) * (f * f * f) + (B + 5.f * C) * (f * f) + (-2.f * B - 8.f * C) * f + (4.f / 3.f * B + 4.f * C);
		}
		return 0.f;
	}

	SColor TImage::GetPixelBicubic(float x, float y, int32 MipIndex)
	{
		int32 xi = (int32)(x);
		int32 yi = (int32)(y);

		float dx = x - xi;
		float dy = y - yi;

		float Bmdx;	// Bspline m-dx 
		float Bndy;	// Bspline dy-n    

		vector4df Result;
		for (int32 m = -1; m <= 2; m++)
		{
			Bmdx = Cubic(m - dx);
			for (int32 n = -1; n <= 2; n++)
			{
				Bndy = Cubic(dy - n);

				SColor c = GetPixel(xi + m, yi + n);
				vector4df r(c.R, c.G, c.B, c.A);
				Result += r * Bmdx * Bndy;
			}
		}

		Result.X = ti_max(0.f, Result.X);
		Result.Y = ti_max(0.f, Result.Y);
		Result.Z = ti_max(0.f, Result.Z);
		Result.W = ti_max(0.f, Result.W);
		Result.X = ti_min(255.f, Result.X);
		Result.Y = ti_min(255.f, Result.Y);
		Result.Z = ti_min(255.f, Result.Z);
		Result.W = ti_min(255.f, Result.W);

		SColor Color;
		Color.R = (uint8)ti_round(Result.X);
		Color.G = (uint8)ti_round(Result.Y);
		Color.B = (uint8)ti_round(Result.Z);
		Color.A = (uint8)ti_round(Result.W);

		return Color;
	}

	SColorf TImage::GetPixelFloat(int32 x, int32 y, int32 MipIndex)
	{
		int32 Width = Mipmaps[MipIndex].W;
		int32 Height = Mipmaps[MipIndex].H;
		int32 Pitch = Mipmaps[MipIndex].RowPitch;
		uint8* Data = (uint8*)Mipmaps[MipIndex].Data.GetBuffer();

		TI_ASSERT(x >= 0);
		TI_ASSERT(y >= 0);
		if (x >= Width)
			x = Width - 1;
		if (y >= Height) 
			y = Height - 1;
		SColorf c;
		int32 offset = y * Pitch + x * GetPixelSizeInBytes(PixelFormat);
		float *fdata = (float*)(Data + offset);
		switch (PixelFormat)
		{
		case EPF_DEPTH32:
			c.R = fdata[0];
			c.G = fdata[0];
			c.B = fdata[0];
			c.A = fdata[0];
			break;
		case EPF_RG32F:
			c.R = fdata[0];
			c.G = fdata[1];
			c.B = 0;
			c.A = 0;
			break;
		case EPF_RGBA32F:
			c.R = fdata[0];
			c.G = fdata[1];
			c.B = fdata[2];
			c.A = fdata[3];
			break;
		default:
			TI_ASSERT(0);
		}
		return c;
	}

	SColorf TImage::GetPixelFloat(float x, float y, int32 MipIndex)
	{
		int32 xi=(int32)(x); if (x<0) xi--;   //these replace (incredibly slow) floor (Visual c++ 2003, AMD Athlon)
		int32 yi=(int32)(y); if (y<0) yi--;

		float t1=x-xi, t2=y-yi;
		float d=t1*t2;
		float b=t1-d;
		float c=t2-d;
		float a=1-t1-c;
		SColorf rgb11,rgb21,rgb12,rgb22;
		rgb11 = GetPixelFloat(xi, yi, MipIndex);
		rgb21 = GetPixelFloat(xi+1, yi, MipIndex);
		rgb12 = GetPixelFloat(xi, yi+1, MipIndex);
		rgb22 = GetPixelFloat(xi+1, yi+1, MipIndex);

		SColorf color;
		//calculate linear interpolation
		color.R	= (a * rgb11.R + b * rgb21.R + c * rgb12.R + d * rgb22.R);
		color.G = (a * rgb11.G + b * rgb21.G + c * rgb12.G + d * rgb22.G);
		color.B = (a * rgb11.B + b * rgb21.B + c * rgb12.B + d * rgb22.B);
		color.A = (a * rgb11.A + b * rgb21.A + c * rgb12.A + d * rgb22.A);

		return color;
	}

	SColorf TImage::GetPixelFloatBicubic(float x, float y, int32 MipIndex)
	{
		int32 xi = (int32)(x);
		int32 yi = (int32)(y);

		float dx = x - xi;
		float dy = y - yi;

		float Bmdx;	// Bspline m-dx 
		float Bndy;	// Bspline dy-n    

		vector4df Result;
		for (int32 m = -1; m <= 2; m++)
		{
			Bmdx = Cubic(m - dx);
			for (int32 n = -1; n <= 2; n++)
			{
				Bndy = Cubic(dy - n);

				SColorf c = GetPixelFloat(xi + m, yi + n);
				vector4df r(c.R, c.G, c.B, c.A);
				Result += r * Bmdx * Bndy;
			}
		}
		
		SColorf Color;
		Color.R = (Result.X);
		Color.G = (Result.Y);
		Color.B = (Result.Z);
		Color.A = (Result.W);

		return Color;
	}

	void TImage::ClearMipmaps()
	{
		// delete mipmaps
		Mipmaps.clear();
	}

	int32 TImage::CalcMipCount(int32 Width, int32 Height)
	{
		int32 MipCount = 0;
		int32 W = Width;
		int32 H = Height;
		while (W > 0 && H > 0)
		{
			++MipCount;
			W /= 2;
			H /= 2;
		}
		return MipCount;
	}

	void TImage::AllocEmptyMipmaps()
	{
		TI_ASSERT(Mipmaps.size() > 0);
		int32 W = Mipmaps[0].W;
		int32 H = Mipmaps[0].H;
		bool IsCompressed = IsCompressedFormat(PixelFormat);

		int32 MipCount = CalcMipCount(W, H);
		Mipmaps.resize(MipCount);

		for (int32 Mip = 0; Mip < MipCount; ++Mip)
		{
			TSurfaceData* MipData = &Mipmaps[Mip];

			if (Mip > 0)
			{
				MipData->W = W;
				MipData->H = H;
				if (IsCompressed)
				{
					vector2di Block = GetBlockSize(PixelFormat);
					MipData->BlockSize = GetBlockSizeInBytes(PixelFormat);
					int32 BlockW = GetBlockWidth(W, Block.X);
					MipData->RowPitch = MipData->BlockSize * BlockW;
				}
				else
				{
					MipData->RowPitch = GetPixelSizeInBytes(PixelFormat) * W;
				}
				MipData->Data.ReserveAndFill(GetDataSize(PixelFormat, W, H));
			}

			W /= 2;
			H /= 2;
		}
	}

	void TImage::GenerateMipmaps(int32 TargetMips)
	{
		TI_ASSERT(Mipmaps.size() > 0);
		int32 W = Mipmaps[0].W;
		int32 H = Mipmaps[0].H;
		bool IsCompressed = IsCompressedFormat(PixelFormat);
		if (IsCompressed)
		{
			return;
		}
		int32 MipCount;
		if (TargetMips > 0)
		{
			MipCount = TargetMips;
		}
		else
		{
			MipCount = CalcMipCount(W, H);
		}

		if (Mipmaps.size() < MipCount)
		{
			AllocEmptyMipmaps();
		}

		// Down sample to generate all mips
		for (int32 Mip = 0 ; Mip < MipCount - 1 ; ++ Mip)
		{
			for (int32 y = 0 ; y < H ; y += 2)
			{
				for (int32 x = 0 ; x < W ; x += 2)
				{
					SColor c00 = GetPixel(x + 0, y + 0, Mip);
					SColor c10 = GetPixel(x + 1, y + 0, Mip);
					SColor c01 = GetPixel(x + 0, y + 1, Mip);
					SColor c11 = GetPixel(x + 1, y + 1, Mip);

					float Rf, Gf, Bf, Af;
					Rf = (float)(c00.R + c10.R + c01.R + c11.R);
					Gf = (float)(c00.G + c10.G + c01.G + c11.G);
					Bf = (float)(c00.B + c10.B + c01.B + c11.B);
					Af = (float)(c00.A + c10.A + c01.A + c11.A);

					// Calc average
					SColor Target;
					Target.R = ti_round(Rf * 0.25f);
					Target.G = ti_round(Gf * 0.25f);
					Target.B = ti_round(Bf * 0.25f);
					Target.A = ti_round(Af * 0.25f);

					// Set to next mip
					SetPixel(x / 2, y / 2, Target, Mip + 1);
				}
			}

			W /= 2;
			H /= 2;
		}
	}

	static const SColor k_mipmap_color[]	= {
		SColor(255, 255, 0, 0),	//level1 : red
		SColor(255, 0, 255, 0),	//level2 : green
		SColor(255, 0, 0, 255),	//level3 : blue
		SColor(255, 255, 255, 0),
		SColor(255, 255, 0, 255),
		SColor(255, 0, 255, 255),
		SColor(255, 128, 128, 128),
		SColor(255, 0, 128, 0),
		SColor(255, 255, 255, 255),
		SColor(255, 255, 255, 255),
		SColor(255, 255, 255, 255),
		SColor(255, 255, 255, 255),
	};

	void TImage::FlipY()
	{
		if (IsCompressedFormat(GetFormat()))
		{
			return;
		}
		for (int32 Mip = 0 ; Mip < (int32)Mipmaps.size() ; ++ Mip)
		{
			int32 Width = Mipmaps[Mip].W;
			int32 Height = Mipmaps[Mip].H;
			int32 Pitch = Mipmaps[Mip].RowPitch;
			uint8* Data = (uint8*)Mipmaps[Mip].Data.GetBuffer();

			uint8* tmp = ti_new uint8[Pitch];
			for (int32 y = 0; y < Height / 2; ++y)
			{
				int32 y0 = y * Pitch;
				int32 y1 = (Height - y - 1) * Pitch;

				memcpy(tmp, Data + y0, Pitch);
				memcpy(Data + y0, Data + y1, Pitch);
				memcpy(Data + y1, tmp, Pitch);
			}
			ti_delete[] tmp;
		}
	}

	bool TImage::CopyRegionTo(TImage* DstImage, const recti& DstRegion, int32 DstMip, const recti& SrcRegion, int32 SrcMip)
	{
		if (DstImage->GetFormat() != PixelFormat)
		{
			return false;
		}
		if (SrcRegion.Right > GetMipmap(SrcMip).W || SrcRegion.Lower > GetMipmap(SrcMip).H)
		{
			return false;
		}
		if (DstRegion.Right > DstImage->GetMipmap(DstMip).W || DstRegion.Lower > DstImage->GetMipmap(DstMip).H)
		{
			return false;
		}
		if (IsCompressedFormat(PixelFormat))
		{
			// Compress format NOT support for now, CAN BE supported in future.
			return false;
		}
		if (DstRegion.getWidth() != SrcRegion.getWidth() || DstRegion.getHeight() != SrcRegion.getHeight())
		{
			float StepX = (float)(SrcRegion.getWidth()) / (float)(DstRegion.getWidth());
			float StepY = (float)(SrcRegion.getHeight()) / (float)(DstRegion.getHeight());

			const int32 DstH = DstRegion.getHeight();
			const int32 DstW = DstRegion.getWidth();

			for (int32 y = 0 ; y < DstH ; ++ y)
			{
				for (int32 x = 0 ; x < DstW ; ++ x)
				{
					float xx = x * StepX;
					float yy = y * StepY;
					SColor c = GetPixelBicubic(SrcRegion.Left + xx, SrcRegion.Upper + yy);
					DstImage->SetPixel(x + DstRegion.Left, y + DstRegion.Upper, c);
				}
			}

			return true;
		}
		else
		{
			// Src.Size == Dst.Size
			const int32 PixelSizeInBytes = GetPixelSizeInBytes(PixelFormat);

			uint8* DstBuffer = DstImage->Lock();
			int32 DstPitch = DstImage->GetMipmap(DstMip).RowPitch;
			DstBuffer += DstRegion.Upper * DstPitch + DstRegion.Left * PixelSizeInBytes;
			int32 DstRowLength = DstRegion.getWidth() * PixelSizeInBytes;

			uint8* SrcBuffer = Lock(SrcMip);
			int32 SrcPitch = GetMipmap(SrcMip).RowPitch;
			SrcBuffer += SrcRegion.Upper * SrcPitch + SrcRegion.Left * PixelSizeInBytes;
			for (int32 y = 0 ; y < DstRegion.getHeight() ; ++ y)
			{
				memcpy(DstBuffer, SrcBuffer, DstRowLength);
				DstBuffer += DstPitch;
				SrcBuffer += SrcPitch;
			}
			DstImage->Unlock();
			Unlock();

			return true;
		}
	}

	void TImage::ConvertToLinearSpace()
	{
		if (IsCompressedFormat(GetFormat()))
		{
			_LOG(Error, "Convert to linear space, not support compress format.\n");
			return;
		}
		for (int32 y = 0 ; y < GetWidth() ; ++ y)
		{
			for (int32 x = 0 ; x < GetHeight() ; ++ x)
			{
				static const float S = 255.f;
				static const float SInv = 1.f / S;
				static const float Gamma = 2.2f;
				SColor C = GetPixel(x, y);
				vector4df R(C.R * SInv, C.G * SInv, C.B * SInv, C.A * SInv);
				R.X = pow(R.X, Gamma);
				R.Y = pow(R.Y, Gamma);
				R.Z = pow(R.Z, Gamma);

				C.R = (uint8)ti_round(R.X * S);
				C.G = (uint8)ti_round(R.Y * S);
				C.B = (uint8)ti_round(R.Z * S);
				C.A = (uint8)ti_round(R.W * S);
				SetPixel(x, y, C);
			}
		}
	}

	void TImage::ConvertToSrgbSpace()
	{
		if (IsCompressedFormat(GetFormat()))
		{
			_LOG(Error, "Convert to srgb space, not support compress format.\n");
			return;
		}
		for (int32 y = 0; y < GetWidth(); ++y)
		{
			for (int32 x = 0; x < GetHeight(); ++x)
			{
				static const float S = 255.f;
				static const float SInv = 1.f / S;
				static const float Gamma = 2.2f;
				static const float GammaInv = 1.f / Gamma;
				SColor C = GetPixel(x, y);
				vector4df R(C.R * SInv, C.G * SInv, C.B * SInv, C.A * SInv);
				R.X = pow(R.X, GammaInv);
				R.Y = pow(R.Y, GammaInv);
				R.Z = pow(R.Z, GammaInv);

				C.R = (uint8)ti_round(R.X * S);
				C.G = (uint8)ti_round(R.Y * S);
				C.B = (uint8)ti_round(R.Z * S);
				C.A = (uint8)ti_round(R.W * S);
				SetPixel(x, y, C);
			}
		}
	}
}
