/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TImage;
	typedef TI_INTRUSIVE_PTR(TImage) TImagePtr;

	class TImage : public IReferenceCounted
	{
	public:
		TI_API static bool	IsCompressedFormat(int fmt);

		TImage(E_PIXEL_FORMAT pixel_format, int w, int h, int data_size = -1);
		virtual ~TImage();

		TI_API static TImagePtr	LoadImageTix(TFile& file_input, bool is_srgb);

		TI_API void			GenerateMipmaps();
		TI_API void			ClearMipmaps();
		TI_API TImagePtr	Clone();
		TI_API void			FlipY();

		TI_API void			SetPixel(int x, int y, const SColor& c);
		TI_API void			SetPixel(int x, int y, uint8 c);
		TI_API void			SetPixel(int x, int y, const SColorf& c);
		TI_API SColor		GetPixel(int x, int y);
		TI_API SColor		GetPixel(float x, float y);
		TI_API SColorf		GetPixelFloat(int x, int y);
		TI_API SColorf		GetPixelFloat(float x, float y);

		virtual uint8*		Lock();
		virtual void		Unlock();

		int32	GetPitch() const
		{
			return Pitch;
		}

		int		GetWidth() const
		{
			return Width;
		}

		int		GetHeight() const
		{
			return Height;
		}

		E_PIXEL_FORMAT	GetFormat() const
		{
			return PixelFormat;
		}

		int		GetMipmapCount() const
		{
			return (int)Mipmaps.size();
		}

		void	AddMipmap(TImagePtr image)
		{
			Mipmaps.push_back(image);
		}
        
        int     GetDataLength() const
        {
            return  DataSize;
        }

		TImagePtr	GetMipmap(int mipmap_level);

	protected:
		E_PIXEL_FORMAT	PixelFormat;
		int		Width;
		int		Height;
		int		Pitch;
		int		DataSize;

		uint8*	Data;

		typedef std::vector<TImagePtr> VecMipmapImages;
		VecMipmapImages		Mipmaps;
	};
}