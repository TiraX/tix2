/*
	Copyright (C) 2012
	By Zhao Shuai tirax.cn@gmail.com 2012.9.04
*/

#include "stdafx.h"
#include "TImage.h"

namespace tix
{
	inline int32 GetPixelSizeInBytes(E_PIXEL_FORMAT Format)
	{
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
		case EPF_RGB16F:
			return 6;
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
		default:
			return 0;
		}
	}

	TImage::TImage(E_PIXEL_FORMAT pixel_format, int32 w, int32 h)
		: PixelFormat(pixel_format)
		, Width(w)
		, Height(h)
	{
		Pitch = Width * GetPixelSizeInBytes(pixel_format);
		TI_ASSERT(Pitch != 0);
		DataSize = Pitch * Height;
		Data = ti_new uint8[DataSize];
		memset(Data, 0, sizeof(uint8) * DataSize);
	}

	TImage::~TImage()
	{
		ClearMipmaps();

		ti_delete[] Data;
	}

	uint8* TImage::Lock()
	{
		return Data;
	}

	void TImage::Unlock()
	{
	}

	int32 TImage::GetPitch()
	{
		return Pitch;
	}

	void TImage::SetPixel(int32 x, int32 y, const SColor& c)
	{
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

	void TImage::SetPixel(int32 x, int32 y, const SColorf& c)
	{
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
		case EPF_RGB16F:
			hdata[0] = half(c.R);
			hdata[1] = half(c.G);
			hdata[2] = half(c.B);
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

	void TImage::SetPixel(int32 x, int32 y, uint8 c)
	{
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

	SColor TImage::GetPixel(int32 x, int32 y)
	{
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

	SColor TImage::GetPixel(float x, float y)
	{
		int32 xi=(int32)(x); if (x<0) xi--;   //these replace (incredibly slow) floor (Visual c++ 2003, AMD Athlon)
		int32 yi=(int32)(y); if (y<0) yi--;

		float t1=x-xi, t2=y-yi;
		float d=t1*t2;
		float b=t1-d;
		float c=t2-d;
		float a=1-t1-c;
		SColor rgb11,rgb21,rgb12,rgb22;
		rgb11 = GetPixel(xi, yi);
		rgb21 = GetPixel(xi+1, yi);
		rgb12 = GetPixel(xi, yi+1);
		rgb22 = GetPixel(xi+1, yi+1);

		SColor color;
		//calculate linear interpolation
		color.R	= (uint8) (a * rgb11.R + b * rgb21.R + c * rgb12.R + d * rgb22.R);
		color.G = (uint8) (a * rgb11.G + b * rgb21.G + c * rgb12.G + d * rgb22.G);
		color.B = (uint8) (a * rgb11.B + b * rgb21.B + c * rgb12.B + d * rgb22.B);
		color.A = (uint8) (a * rgb11.A + b * rgb21.A + c * rgb12.A + d * rgb22.A);

		return color;
	}

	SColorf TImage::GetPixelFloat(int32 x, int32 y)
	{
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

	SColorf TImage::GetPixelFloat(float x, float y)
	{
		int32 xi=(int32)(x); if (x<0) xi--;   //these replace (incredibly slow) floor (Visual c++ 2003, AMD Athlon)
		int32 yi=(int32)(y); if (y<0) yi--;

		float t1=x-xi, t2=y-yi;
		float d=t1*t2;
		float b=t1-d;
		float c=t2-d;
		float a=1-t1-c;
		SColorf rgb11,rgb21,rgb12,rgb22;
		rgb11 = GetPixelFloat(xi, yi);
		rgb21 = GetPixelFloat(xi+1, yi);
		rgb12 = GetPixelFloat(xi, yi+1);
		rgb22 = GetPixelFloat(xi+1, yi+1);

		SColorf color;
		//calculate linear interpolation
		color.R	= (a * rgb11.R + b * rgb21.R + c * rgb12.R + d * rgb22.R);
		color.G = (a * rgb11.G + b * rgb21.G + c * rgb12.G + d * rgb22.G);
		color.B = (a * rgb11.B + b * rgb21.B + c * rgb12.B + d * rgb22.B);
		color.A = (a * rgb11.A + b * rgb21.A + c * rgb12.A + d * rgb22.A);

		return color;
	}

	void TImage::ClearMipmaps()
	{
		// delete mipmaps
		for (int32 i = 0 ; i < (int32)Mipmaps.size() ; ++ i)
		{
			ti_delete Mipmaps[i];
		}
		Mipmaps.clear();
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
		uint8* tmp = ti_new uint8[Pitch];
		for (int32 y = 0 ; y < Height / 2 ; ++ y)
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
