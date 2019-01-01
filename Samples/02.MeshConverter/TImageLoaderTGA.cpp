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
					BGRA8_TO_RGBA8(src, dst);
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
		else
		{
			TI_ASSERT(0);
		}
	}

	TImage* TImage::LoadImageTGA(TFile& FileInput)
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
				dstFmt = EPF_RGB8;
			}
			break;

			case 32:
			{
				srcFmt = EPF_BGRA8;
				dstFmt = EPF_RGBA8;	//need convert.
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

		TImage* image = ti_new TImage(dstFmt, header.ImageWidth, header.ImageHeight);
		if (image)
		{
			void* src = image->Lock();

			// read image
			if (header.ImageType == 2)
			{
				const int imageSize = header.ImageHeight * header.ImageWidth * header.PixelDepth / 8;
				FileInput.Read(src, imageSize, imageSize);
				ConvertPixelFormat((unsigned char*)src, (unsigned char*)src,
					dstFmt, srcFmt,
					header.ImageWidth, header.ImageHeight);
			}
			else if (header.ImageType == 3)
			{
				const int32 imageSize = header.ImageHeight * header.ImageWidth * header.PixelDepth / 8;
				FileInput.Read(src, imageSize, imageSize);
			}
			else //if (header.ImageType == 10)
			{
				TI_ASSERT(0);
				printf("Compressed TGA format is not supported yet.");
			}

			image->Unlock();
		}

		return image;
	}

	bool TImage::SaveToTga(const char* filename)
	{
		FILE *fp;
		fp	= fopen(filename, "wb");
		if (fp == NULL)
		{
			return false;
		}

		STGAHeader header;
		memset(&header, 0, sizeof(STGAHeader));
		header.ImageType = 2;
		header.ImageWidth = Width;
		header.ImageHeight = Height;
		header.PixelDepth = GetFormat() == EPF_RGB8 ? 24 : 32;
		header.ImageDescriptor = 8;

		fwrite( &header, sizeof(STGAHeader) , 1, fp);

		TStream pixelBuffer;
		uint8 pixelfmt[4];

		const int32 image_size	= Width * Height;

		// convert RGBA to BGRA
		if (GetFormat() == EPF_RGB8)
		{
			for( int32 i = 0; i < image_size; i++ )
			{
				pixelfmt[0]	= Data[i * 3 + 2];
				pixelfmt[1]	= Data[i * 3 + 1];
				pixelfmt[2]	= Data[i * 3 + 0];

				pixelBuffer.Put(pixelfmt, 3);
			}
		}
		else
		{
			for( int32 i = 0; i < image_size; i++ )
			{
				pixelfmt[0]	= Data[i * 4 + 2];
				pixelfmt[1]	= Data[i * 4 + 1];
				pixelfmt[2]	= Data[i * 4 + 0];
				pixelfmt[3]	= Data[i * 4 + 3];

				pixelBuffer.Put(pixelfmt, 4);
			}
		}


		fwrite( pixelBuffer.GetBuffer(),pixelBuffer.GetLength(), 1, fp);
		fclose(fp);

		return true;
	}
}
