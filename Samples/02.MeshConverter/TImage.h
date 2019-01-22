/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TImage
	{
	public:
		TImage(E_PIXEL_FORMAT pixel_format, int32 w, int32 h);
		virtual ~TImage();

		static TImage* LoadImageTGA(TFile& FileInput);
		bool SaveToTga(const char* filename);

		void FlipY();
		void ClearMipmaps();

		void SetPixel(int32 x, int32 y, const SColor& c);
		void SetPixel(int32 x, int32 y, uint8 c);
		void SetPixel(int32 x, int32 y, const SColorf& c);
		SColor GetPixel(int32 x, int32 y);
		SColor GetPixel(float x, float y);
		SColorf GetPixelFloat(int32 x, int32 y);
		SColorf GetPixelFloat(float x, float y);

		virtual uint8* Lock();
		virtual void Unlock();

		virtual int32 GetPitch();

		int32 GetWidth()
		{
			return Width;
		}

		int32 GetHeight()
		{
			return Height;
		}

		E_PIXEL_FORMAT GetFormat()
		{
			return PixelFormat;
		}

		int32 GetMipmapCount()
		{
			return (int32)Mipmaps.size();
		}

		void AddMipmap(TImage* image)
		{
			Mipmaps.push_back(image);
		}
        
        int32 GetDataLength()
        {
            return  DataSize;
        }

		TImage*GetMipmap(int32 mipmap_level);

	protected:
		E_PIXEL_FORMAT PixelFormat;
		int32 Width;
		int32 Height;
		int32 Pitch;
		int32 DataSize;

		uint8* Data;

		typedef std::vector<TImage*> VecMipmapImages;
		VecMipmapImages Mipmaps;
	};
}