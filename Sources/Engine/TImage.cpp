/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TImage.h"

namespace tix
{
	static const int32 k_PIXEL_FORMAT_SIZE_MAP[EPF_COUNT] =
	{
		1,	//EPF_A8,
		3,	//EPF_RGB8,
		3,	//EPF_BGR8,
		4,	//EPF_RGBA8,
		4,	//EPF_BGRA8,
		3,	//EPF_SRGB8,
		4,	//EPF_SRGB8_ALPHA,

		// Float formats
		2,	//EPF_R16F,
		4,	//EPF_RG16F,
		6,	//EPF_RGB16F,
		8,	//EPF_RGBA16F,
		4,	//EPF_R32F,
		8,	//EPF_RG32F,
		12,	//EPF_RGB32F,
		16,	//EPF_RGBA32F,

		// Depth formats
		2,	//EPF_DEPTH16,
		3,	//EPF_DEPTH24,
		4,	//EPF_DEPTH32,
		4,	//EPF_DEPTH24_STENCIL8,

		// DXT formats
		// for compressed formats, we use block size to calc data size
		// a block is a 4x4 pixels rect.
		// DXT formats (compressed)
		8,	//EPF_COMPRESSED_RGB_S3TC_DXT1,
		8,	//EPF_COMPRESSED_RGBA_S3TC_DXT1,
		16,	//EPF_COMPRESSED_RGBA_S3TC_DXT3,
		16,	//EPF_COMPRESSED_RGBA_S3TC_DXT5,

		// PVR formats
		8,	//EPF_COMPRESSED_sRGB_PVRTC_4BPP,
		8,	//EPF_COMPRESSED_sRGB_Alpha_PVRTC_4BPP,
		8,	//EPF_COMPRESSED_RGB_PVRTC_4BPP,
		8,	//EPF_COMPRESSED_RGBA_PVRTC_4BPP,

		// ETC format
		-1,	//EPF_COMPRESSED_ETC,

		-1	//EPF_UNKNOWN
	};

	TImage::TImage(E_PIXEL_FORMAT pixel_format, int w, int h, int data_size)
		: PixelFormat(pixel_format)
		, Width(w)
		, Height(h)
	{
        if (data_size != -1)
        {
            DataSize        = data_size;
            Data            = ti_new uint8[DataSize];
            memset(Data, 0, DataSize);
            Pitch           = 0;
        }
        else
        {
            if (IsCompressedFormat(PixelFormat))
            {
                DataSize	= ((w + 3) / 4) * ((h + 3) / 4) * k_PIXEL_FORMAT_SIZE_MAP[pixel_format];
                Data		= ti_new uint8[DataSize];
                memset(Data, 0, sizeof(uint8) * DataSize );
                Pitch		= 0;
            }
            else
            {
                Pitch		= Width * k_PIXEL_FORMAT_SIZE_MAP[pixel_format];
                DataSize	= Pitch * Height;
                Data		= ti_new uint8[DataSize];
                memset(Data, 0, sizeof(uint8) * DataSize );
            }
        }
	}

	TImage::~TImage()
	{
		ClearMipmaps();

		ti_delete[] Data;
	}

	bool TImage::IsCompressedFormat(int fmt)
	{
		bool IsCompressed = fmt >= EPF_COMPRESSED_RGB_S3TC_DXT1 && fmt < EPF_UNKNOWN;
		return IsCompressed;
	}

	TImagePtr TImage::Clone()
	{
		TImagePtr cImage	= ti_new TImage(PixelFormat, Width, Height);
		memcpy(cImage->Lock(), Data, Height * Pitch);
		cImage->Unlock();

		return cImage;
	}

	uint8* TImage::Lock()
	{
		return Data;
	}

	void TImage::Unlock()
	{
	}

	TImagePtr TImage::GetMipmap(int mipmap_level)
	{
		TI_ASSERT(mipmap_level >= 0 && mipmap_level < (int)Mipmaps.size());
		return	Mipmaps[mipmap_level];
	}

	void TImage::SetPixel(int x, int y, const SColor& c)
	{
		TI_ASSERT(!IsCompressedFormat(PixelFormat));
		int offset	= y * Pitch + x * k_PIXEL_FORMAT_SIZE_MAP[PixelFormat];
		switch (PixelFormat)
		{
		case EPF_SRGB8_ALPHA:
		case EPF_RGBA8:
			Data[offset + 0]	= c.R;
			Data[offset + 1]	= c.G;
			Data[offset + 2]	= c.B;
			Data[offset + 3]	= c.A;
			break;
		case EPF_SRGB8:
		case EPF_RGB8:
			Data[offset + 0]	= c.R;
			Data[offset + 1]	= c.G;
			Data[offset + 2]	= c.B;
			break;
		default:
			TI_ASSERT(0);
			break;
		}
	}

	void TImage::SetPixel(int x, int y, const SColorf& c)
	{
		TI_ASSERT(!IsCompressedFormat(PixelFormat));
		int offset		= y * Pitch + x * k_PIXEL_FORMAT_SIZE_MAP[PixelFormat];
		float* fdata	= (float*)(Data + offset);
		switch (PixelFormat)
		{
		case EPF_RG32F:
			fdata[0]			= c.R;
			fdata[1]			= c.G;
			break;
		case EPF_RGBA32F:
			fdata[0]			= c.R;
			fdata[1]			= c.G;
			fdata[2]			= c.B;
			fdata[3]			= c.A;
			break;
		default:
			TI_ASSERT(0);
			break;
		}
	}

	void TImage::SetPixel(int x, int y, uint8 c)
	{
		TI_ASSERT(!IsCompressedFormat(PixelFormat));
		int offset	= y * Pitch + x * k_PIXEL_FORMAT_SIZE_MAP[PixelFormat];
		switch (PixelFormat)
		{
		case EPF_A8:
			Data[offset]		= c;
			break;
		default:
			TI_ASSERT(0);
			break;
		}
	}

	SColor TImage::GetPixel(int x, int y)
	{
		TI_ASSERT(!IsCompressedFormat(PixelFormat));
		if ( x >= Width) x = Width - 1;
		if ( y >= Height) y = Height - 1;
		if ( x < 0) x = 0;
		if ( y < 0) y = 0;

		SColor color;
		int offset	= y * Pitch + x * k_PIXEL_FORMAT_SIZE_MAP[PixelFormat];
		switch (PixelFormat)
		{
		case EPF_SRGB8_ALPHA:
		case EPF_RGBA8:
			{
				color.R		= Data[offset + 0];
				color.G		= Data[offset + 1];
				color.B		= Data[offset + 2];
				color.A		= Data[offset + 3];
			}
			break;
		case EPF_SRGB8:
		case EPF_RGB8:
			{
				color.R		= Data[offset + 0];
				color.G		= Data[offset + 1];
				color.B		= Data[offset + 2];
				color.A		= 255;
			}
			break;
		case EPF_A8:
			{
				color.R		= Data[offset];
				color.G		= Data[offset];
				color.B		= Data[offset];
				color.A		= Data[offset];
			}
			break;
		default:
			TI_ASSERT(0);
		}
		return color;
	}

	SColor TImage::GetPixel(float x, float y)
	{
		TI_ASSERT(!IsCompressedFormat(PixelFormat));
		int xi=(int)(x); if (x<0) xi--;   //these replace (incredibly slow) floor (Visual c++ 2003, AMD Athlon)
		int yi=(int)(y); if (y<0) yi--;

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

	SColorf TImage::GetPixelFloat(int x, int y)
	{
		TI_ASSERT(!IsCompressedFormat(PixelFormat));
		TI_ASSERT(x >= 0);
		TI_ASSERT(y >= 0);
		if (x >= Width)		x	= Width - 1;
		if (y >= Height)	y	= Height - 1;
		SColorf c;
		int offset		= y * Pitch + x * k_PIXEL_FORMAT_SIZE_MAP[PixelFormat];
		float *fdata	= (float*)(Data + offset);
		switch (PixelFormat)
		{
		case EPF_DEPTH32:
			c.R				= fdata[0];
			c.G				= fdata[0];
			c.B				= fdata[0];
			c.A				= fdata[0];
			break;
		case EPF_RG32F:
			c.R				= fdata[0];
			c.G				= fdata[1];
			c.B				= 0;
			c.A				= 0;
			break;
		case EPF_RGBA32F:
			c.R				= fdata[0];
			c.G				= fdata[1];
			c.B				= fdata[2];
			c.A				= fdata[3];
			break;
		default:
			TI_ASSERT(0);
		}
		return c;
	}

	SColorf TImage::GetPixelFloat(float x, float y)
	{
		TI_ASSERT(!IsCompressedFormat(PixelFormat));
		// map coord to 0.f to 1.f
		//if (x > 0.f)
		//	x	= x - (int)x;
		//else if (x < 0.f)
		//	x	= 1.f + (x - (int)x);
		//if (y > 0.f)
		//	y	= y - (int)y;
		//else if (y < 0.f)
		//	y	= 1.f + (y - (int)y);

		int xi=(int)(x); if (x<0) xi--;   //these replace (incredibly slow) floor (Visual c++ 2003, AMD Athlon)
		int yi=(int)(y); if (y<0) yi--;

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
		for (unsigned int i = 0 ; i < Mipmaps.size() ; ++ i)
		{
			Mipmaps[i]	= NULL;
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

	void TImage::GenerateMipmaps()
	{
		if (Mipmaps.size() > 0)
			return;

		if (IsCompressedFormat(PixelFormat))
			return;

		ClearMipmaps();

		int mw		= Width / 2;
		int mh		= Height / 2;
		TImagePtr	SampleImage	= this;
		int index	= 0;
		while (mw >= 1 && mh >= 1)
		{
			TImagePtr mipmap	= ti_new TImage(PixelFormat, mw, mh);
			
			if (PixelFormat == EPF_RGBA32F)
			{
				for (int y = 0; y < mh; ++y)
				{
					for (int x = 0; x < mw; ++x)
					{
						SColorf c = SampleImage->GetPixelFloat(x * 2.0f, y * 2.0f);
						mipmap->SetPixel(x, y, c);
					}
				}
			}
			else
			{
				for (int y = 0; y < mh; ++y)
				{
					for (int x = 0; x < mw; ++x)
					{
						SColor c = SampleImage->GetPixel(x * 2.0f, y * 2.0f);
						mipmap->SetPixel(x, y, c);
					}
				}
			}

			++ index;

			Mipmaps.push_back(mipmap);

			//char mipmap_name[256];
			//sprintf(mipmap_name, "mipmap_%d_%d.png", mw, mh);
			//mipmap->SaveToPng(mipmap_name);

			mw			/= 2;
			mh			/= 2;
			SampleImage	= mipmap;
		}
	}

	void TImage::FlipY()
	{
		if (IsCompressedFormat(PixelFormat))
		{
			return;
		}

		uint8* tmp		= ti_new uint8[Pitch];
		for (int y = 0 ; y < Height / 2 ; ++ y)
		{
			int y0	= y * Pitch;
			int y1	= (Height - y - 1) * Pitch;

			memcpy(tmp, Data + y0, Pitch);
			memcpy(Data + y0, Data + y1, Pitch);
			memcpy(Data + y1, tmp, Pitch);
		}
		ti_delete[] tmp;
	}
}
