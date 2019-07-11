/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TImage : public IReferenceCounted
	{
	public:
		TImage(E_PIXEL_FORMAT InPixelFormat, int32 Width, int32 Height);
		~TImage();
		static int32 GetPixelSizeInBytes(E_PIXEL_FORMAT Format);
		static int32 GetBlockSizeInBytes(E_PIXEL_FORMAT Format);
		static vector2di GetBlockSize(E_PIXEL_FORMAT Format);
		static int32 GetDataSize(E_PIXEL_FORMAT Format, int32 Width, int32 Height);
		static int32 GetRowPitch(E_PIXEL_FORMAT Format, int32 Width);
		static bool IsCompressedFormat(E_PIXEL_FORMAT Format);
		static int32 CalcMipCount(int32 Width, int32 Height);

		static TImage* LoadImageTGA(TFile& FileInput, int32* PixelDepth = nullptr);
		static vector2di LoadImageTGADimension(const TString& TgaName);
		bool SaveToTga(const char* filename, int32 MipIndex = 0);

		void FlipY();
		void ClearMipmaps();
		bool CopyRegionTo(TImage* DstImage, const recti& DstRegion, int32 DstMip, const recti& SrcRegion, int32 SrcMip);

		struct TSurfaceData
		{
			int32 W, H;
			int32 RowPitch;
			int32 BlockSize;
			TStream Data;

			TSurfaceData()
				: W(0)
				, H(0)
				, RowPitch(0)
				, BlockSize(0)
			{}
		};

		void SetPixel(int32 x, int32 y, const SColor& c, int32 MipIndex = 0);
		void SetPixel(int32 x, int32 y, uint8 c, int32 MipIndex = 0);
		void SetPixel(int32 x, int32 y, const SColorf& c, int32 MipIndex = 0);
		SColor GetPixel(int32 x, int32 y, int32 MipIndex = 0);
		SColor GetPixel(float x, float y, int32 MipIndex = 0);
		SColorf GetPixelFloat(int32 x, int32 y, int32 MipIndex = 0);
		SColorf GetPixelFloat(float x, float y, int32 MipIndex = 0);

		virtual uint8* Lock(int32 MipIndex = 0);
		virtual void Unlock();

		void AllocEmptyMipmaps();
		void GenerateMipmaps(int32 TargetMips = 0);

		virtual int32 GetPitch() const
		{
			return Mipmaps[0].RowPitch;
		}

		int32 GetWidth() const
		{
			return Mipmaps[0].W;
		}

		int32 GetHeight() const
		{
			return Mipmaps[0].H;
		}

		E_PIXEL_FORMAT GetFormat() const
		{
			return PixelFormat;
		}

		int32 GetMipmapCount() const
		{
			return (int32)Mipmaps.size();
		}

		const TSurfaceData& GetMipmap(int32 MipLevel) const
		{
			return Mipmaps[MipLevel];
		}

		TSurfaceData& GetMipmap(int32 MipLevel)
		{
			return Mipmaps[MipLevel];
		}
	protected:
		E_PIXEL_FORMAT PixelFormat;

		typedef TVector<TSurfaceData> VecMipmapImages;
		VecMipmapImages Mipmaps;
	};
}