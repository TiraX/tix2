/*
	Copyright (C) 2012
	By Zhao Shuai tirax.cn@gmail.com 2012.9.04
*/

#include "stdafx.h"
#include "TImage.h"

namespace tix
{
	// byte-align structures
#if defined(_MSC_VER) 
#	pragma pack( push, packing )
#	pragma pack( 1 )
#	pragma warning(disable:4103)
#elif defined(__ARMCC_VERSION) //RVCT compiler
#	pragma push
#   pragma pack( 1 )
#elif defined(TI_PLATFORM_IOS)
#   pragma push
#   pragma pack( 1 )
#endif

	struct STGAHeader
	{
		uint8 IdLength;
		uint8 ColorMapType;
		uint8 ImageType;
		uint8 FirstEntryIndex[2];
		uint16 ColorMapLength;
		uint8 ColorMapEntrySize;
		uint8 XOrigin[2];
		uint8 YOrigin[2];
		uint16 ImageWidth;
		uint16 ImageHeight;
		uint8 PixelDepth;
		uint8 ImageDescriptor;
	} ;

	struct STGAFooter
	{
		uint32 ExtensionOffset;
		uint32 DeveloperOffset;
		int8 Signature[18];
	} ;

	// Default alignment
#if defined(_MSC_VER)
#	pragma pack( pop, packing )
#elif defined(__ARMCC_VERSION) //RVCT compiler
#	pragma pop
#elif defined(TI_PLATFORM_IOS) //RVCT compiler
#	pragma pop
#endif


	enum TTgaType
	{
		TGA_COLORMAP_NONE = 0x00,
		/* other color map type codes are reserved */

		TGA_IMAGETYPE_NONE = 0,		/* no image data */
		TGA_IMAGETYPE_PSEUDOCOLOR = 1,	/* color-mapped image */
		TGA_IMAGETYPE_TRUECOLOR = 2,	/* true-color image */
		TGA_IMAGETYPE_GREYSCALE = 3,	/* true-color single channel */
		TGA_IMAGETYPE_RLE_PSEUDOCOLOR = 9,	/* RLE color-mapped image */
		TGA_IMAGETYPE_RLE_TRUECOLOR = 10,	/* RLE true color */
		TGA_IMAGETYPE_RLE_GREYSCALE = 11,	/* RLE true grey */

		HTGA_IMAGETYPE_TRUECOLOR = 0x82,
		HTGA_IMAGETYPE_GREYSCALE = 0x83,
	};

	void GetTgaTypeAndPixelBits(E_PIXEL_FORMAT Format, TTgaType& TgaType, int32& PixelBits)
	{
		switch (Format)
		{
		case EPF_A8:
			TgaType = TGA_IMAGETYPE_GREYSCALE;
			PixelBits = 8;
			break;
		case EPF_RGB8:
			TgaType = TGA_IMAGETYPE_TRUECOLOR;
			PixelBits = 24;
			break;
		case EPF_RGBA8:
			TgaType = TGA_IMAGETYPE_TRUECOLOR;
			PixelBits = 32;
			break;
		case EPF_RGBA16F:
			TgaType = HTGA_IMAGETYPE_TRUECOLOR;
			PixelBits = 64;
			break;
		case EPF_R16F:
			TgaType = HTGA_IMAGETYPE_GREYSCALE;
			PixelBits = 16;
			break;
		case EPF_R32F:
			TgaType = HTGA_IMAGETYPE_GREYSCALE;
			PixelBits = 32;
			break;
		default:
			TI_ASSERT(0);
			break;
		}
	}


#define ARGB1565_TO_RGBA8(pSrc, pDst)	()
#define BGRA8_TO_RGBA8(pSrc, pDst)	{\
									uint8 tmp = *pSrc;\
									*pSrc = *(pDst + 2);\
									*(pDst + 2) = tmp;\
									}
#define BGR8_TO_RGB8(pSrc, pDst)	{\
									uint8 tmp = *pSrc;\
									*pSrc = *(pDst + 2);\
									*(pDst + 2)	= tmp;\
									}
#define BGR8_TO_RGBA8(pSrc, pDst)	{\
									uint8 tmp = *pSrc;\
									*pSrc = *(pDst + 2);\
									*(pDst + 2)	= tmp;\
									*(pDst + 3) = 255;\
									}


	void ConvertPixelFormat(uint8* dst, uint8* src,
		E_PIXEL_FORMAT dstFmt, E_PIXEL_FORMAT srcFmt,
		int32 w, int32 h)
	{
		if (srcFmt	== EPF_BGRA8 &&
			(dstFmt	== EPF_RGBA8_SRGB || dstFmt == EPF_RGBA8))
		{
			for (int32 y = h - 1 ; y >= 0 ; -- y)
			{
				for (int32 x = w - 1 ; x >= 0 ; --x)
				{
					dst[0] = src[2];
					dst[1] = src[1];
					dst[2] = src[0];
					dst[3] = src[3];
					dst	+= 4;
					src	+= 4;
				}
			}
		}
		else if (srcFmt == EPF_BGR8 &&
				 (dstFmt == EPF_RGB8))
		{
			for (int32 y = h - 1 ; y >= 0 ; -- y)
			{
				for (int32 x = w - 1 ; x >= 0 ; --x)
				{
					BGR8_TO_RGB8(src, dst);
					dst	+= 3;
					src	+= 3;
				}
			}
		}
		else if (srcFmt == EPF_BGR8 &&
				(dstFmt == EPF_RGBA8))
		{
			for (int32 y = h - 1; y >= 0; --y)
			{
				for (int32 x = w - 1; x >= 0; --x)
				{
					dst[0] = src[2];
					dst[1] = src[1];
					dst[2] = src[0];
					dst[3] = 255;
					dst += 4;
					src += 3;
				}
			}
		}
		else
		{
			TI_ASSERT(0);
		}
	}

	TImage* TImage::LoadImageTGA(TFile& FileInput, int32* PixelDepth)
	{
		STGAHeader header;

		FileInput.Read(&header, sizeof(STGAHeader), sizeof(STGAHeader));

		// skip image identification field
		if (header.IdLength)
			FileInput.Seek(header.IdLength, true);

		if (header.ColorMapType)
		{
			// skip color map
			FileInput.Seek(header.ColorMapEntrySize / 8 * header.ColorMapLength, true);
		}
		if (PixelDepth != nullptr)
		{
			*PixelDepth = header.PixelDepth;
		}

		E_PIXEL_FORMAT srcFmt = EPF_UNKNOWN;
		E_PIXEL_FORMAT dstFmt = EPF_UNKNOWN;
		switch (header.PixelDepth)
		{
			case 8:
			{
				srcFmt = EPF_A8;
				dstFmt = EPF_A8;
			}
			break;

			case 24:
			{
				srcFmt = EPF_BGR8;
				dstFmt = EPF_RGBA8;	// Always convert to 32 bit
			}
			break;

			case 32:
			{
				srcFmt = EPF_BGRA8;
				dstFmt = EPF_RGBA8;	// need convert.
			}
			break;

			default:
			{
				printf("Error: Unsupported TGA format. [%s]\n", FileInput.GetFileName().c_str());
				return NULL;
			}
		}

		if (!(header.ImageType == 2 || header.ImageType == 3 || header.ImageType == 10))
		{
			printf("Error: Unsupported TGA file type. [%s]\n", FileInput.GetFileName().c_str());
			return NULL;
		}

		TImage* Image = ti_new TImage(dstFmt, header.ImageWidth, header.ImageHeight);
		if (Image)
		{
			// read image
			if (header.ImageType == TGA_IMAGETYPE_TRUECOLOR)
			{
				const int ImageSize = header.ImageHeight * header.ImageWidth * header.PixelDepth / 8;
				char* SrcBuffer = ti_new char[ImageSize];
				FileInput.Read(SrcBuffer, ImageSize, ImageSize);
				ConvertPixelFormat((unsigned char*)Image->Lock(0), (unsigned char*)SrcBuffer,
					dstFmt, srcFmt,
					header.ImageWidth, header.ImageHeight);
				ti_delete[] SrcBuffer;
			}
			else if (header.ImageType == TGA_IMAGETYPE_GREYSCALE)
			{
				const int32 ImageSize = header.ImageHeight * header.ImageWidth * header.PixelDepth / 8;
				char* SrcBuffer = ti_new char[ImageSize];
				FileInput.Read(SrcBuffer, ImageSize, ImageSize);
				Image->GetMipmap(0).Data.Put(SrcBuffer, ImageSize);
				ti_delete[] SrcBuffer;
			}
			else //if (header.ImageType == 10)
			{
				TI_ASSERT(0);
				printf("Compressed TGA format is not supported yet.");
			}

			Image->Unlock();
		}

		return Image;
	}

	bool TImage::SaveToTga(const char* filename, int32 MipIndex)
	{
		int32 Width = Mipmaps[MipIndex].W;
		int32 Height = Mipmaps[MipIndex].H;
		int32 Pitch = Mipmaps[MipIndex].RowPitch;
		uint8* Data = (uint8*)Mipmaps[MipIndex].Data.GetBuffer();

		FILE *fp;
		fp	= fopen(filename, "wb");
		if (fp == NULL)
		{
			return false;
		}

		TTgaType TgaType;
		int32 BitsPerPixel;
		GetTgaTypeAndPixelBits(GetFormat(), TgaType, BitsPerPixel);

		STGAHeader header;
		memset(&header, 0, sizeof(STGAHeader));
		header.ImageType = TgaType;
		header.ImageWidth = Width;
		header.ImageHeight = Height;
		header.PixelDepth = BitsPerPixel;
		header.ImageDescriptor = 8;

		fwrite( &header, sizeof(STGAHeader) , 1, fp);

		TStream pixelBuffer;
		uint8 PixelLDR[4];
		float PixelFloat[4];
		half PixelHalf[4];

		const int32 image_size	= Width * Height;

		float* FData = (float*)Data;
		half* HData = (half*)Data;

		// convert RGBA to BGRA
		if (GetFormat() == EPF_RGB8)
		{
			for( int32 i = 0; i < image_size; i++ )
			{
				PixelLDR[0]	= Data[i * 3 + 2];
				PixelLDR[1]	= Data[i * 3 + 1];
				PixelLDR[2]	= Data[i * 3 + 0];

				pixelBuffer.Put(PixelLDR, 3);
			}
		}
		else if (GetFormat() == EPF_RGBA8)
		{
			for( int32 i = 0; i < image_size; i++ )
			{
				PixelLDR[0]	= Data[i * 4 + 2];
				PixelLDR[1]	= Data[i * 4 + 1];
				PixelLDR[2]	= Data[i * 4 + 0];
				PixelLDR[3]	= Data[i * 4 + 3];

				pixelBuffer.Put(PixelLDR, 4);
			}
		}
		else if (GetFormat() == EPF_A8)
		{
			for( int32 i = 0; i < image_size; i++ )
			{
				PixelLDR[0]	= Data[i];

				pixelBuffer.Put(PixelLDR, 1);
			}
		}
		else if (GetFormat() == EPF_RGBA16F)
		{
			for (int32 i = 0; i < image_size; i++)
			{
				PixelHalf[0] = HData[i * 4 + 2];
				PixelHalf[1] = HData[i * 4 + 1];
				PixelHalf[2] = HData[i * 4 + 0];
				PixelHalf[3] = HData[i * 4 + 3];

				pixelBuffer.Put(PixelHalf, 4 * sizeof(half));
			}
		}
		else if (GetFormat() == EPF_R16F)
		{
			for (int32 i = 0; i < image_size; i++)
			{
				PixelHalf[0] = HData[i];

				pixelBuffer.Put(PixelHalf, 1 * sizeof(half));
			}
		}
		else if (GetFormat() == EPF_R32F)
		{
			for (int32 i = 0; i < image_size; i++)
			{
				PixelFloat[0] = FData[i];

				pixelBuffer.Put(PixelFloat, 1 * sizeof(float));
			}
		}
		else
		{
			TI_ASSERT(0);
		}


		fwrite( pixelBuffer.GetBuffer(),pixelBuffer.GetLength(), 1, fp);
		fclose(fp);

		return true;
	}
}
