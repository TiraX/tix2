/*
	Copyright (C) 2012
	By Zhao Shuai tirax.cn@gmail.com 2012.9.04
*/

#include "stdafx.h"
#include "TImage.h"
//extern "C" {
#include "png.h"
#include "pngpriv.h"
//}

#ifdef TI_PLATFORM_WIN32
#	ifdef TIX_DEBUG
#pragma comment (lib, "libpng16_debug.lib")
#pragma comment (lib, "zlib_debug.lib")
#	else
#pragma comment (lib, "libpng16")
#pragma comment (lib, "zlib")
#	endif
#else
#error("Unknown platform for libpng library.")
#endif

namespace tix
{
	// PNG function for file reading
	void user_read_data_fcn(png_structp png_ptr, png_bytep data, size_t length)
	{
		png_size_t check;

		// changed by zola {
		TFile* file = (TFile*)png_ptr->io_ptr;
		check = (png_size_t)file->Read((void*)data, (int32)length, (int32)length);
		// }

		if (check != length)
			png_error(png_ptr, "Read Error");
	}

	TImagePtr TImage::LoadImagePNG(TFile& FileInput)
	{
		FileInput.Seek(0, false);

		png_byte buffer[8];
		// Read the first few bytes of the PNG file
		if (FileInput.Read(buffer, 8, 8) != 8)
		{
			_LOG(Error, "load png, can't read file [%s].\n", FileInput.GetFileName().c_str());
			return nullptr;
		}

		// Check if it really is a PNG file
		if (png_sig_cmp(buffer, 0, 8))
		{
			_LOG(Error, "load png [%s] is not a png file.\n", FileInput.GetFileName().c_str());
			return 0;
		}

		// Allocate the png read struct
		png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
			NULL,
			NULL,
			NULL);

		if (!png_ptr)
		{
			_LOG(Error, "load png internal PNG create read struct failure. [%s]\n",
				FileInput.GetFileName().c_str());
			return 0;
		}

		// Allocate the png info struct
		png_infop info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr)
		{
			_LOG(Error, "Error: load png internal PNG create info struct failure. [%s]\n",
				FileInput.GetFileName().c_str());
			png_destroy_read_struct(&png_ptr, NULL, NULL);
			return 0;
		}

		// for proper error handling
		if (setjmp(png_jmpbuf(png_ptr)))
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			return 0;
		}

		// changed by zola so we don't need to have public FILE pointers
		png_set_read_fn(png_ptr, &FileInput, user_read_data_fcn);

		png_set_sig_bytes(png_ptr, 8); // Tell png that we read the signature

		png_read_info(png_ptr, info_ptr); // Read the info section of the png file

		uint32 width;
		uint32 height;
		int32 bitDepth;
		int32 colorType;
		{
			// Use temporary variables to avoid passing casted pointers
			png_uint_32 w, h;
			// Extract info
			png_get_IHDR(png_ptr,
				info_ptr,
				&w, &h,
				(int*)&bitDepth,
				(int*)&colorType,
				NULL,
				NULL,
				NULL);
			width = w;
			height = h;
		}

		// Convert palette color to true color
		if (colorType == PNG_COLOR_TYPE_PALETTE)
		{
			png_set_palette_to_rgb(png_ptr);
		}

		// Convert low bit colors to 8 bit colors
		if (bitDepth < 8)
		{
			if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
			{
				png_set_expand_gray_1_2_4_to_8(png_ptr);
			}
			else
			{
				png_set_packing(png_ptr);
			}
		}

		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		{
			png_set_tRNS_to_alpha(png_ptr);
		}

		// Convert high bit colors to 8 bit colors
		if (bitDepth == 16)
		{
			png_set_strip_16(png_ptr);
		}

		// Convert gray color to true color
		if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
		{
			png_set_gray_to_rgb(png_ptr);
		}

		// Update the changes
		png_read_update_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr,
			(png_uint_32*)&width,
			(png_uint_32*)&height,
			(int*)&bitDepth,
			(int*)&colorType,
			NULL,
			NULL,
			NULL);

		E_PIXEL_FORMAT pixelformat;

		// Convert RGBA to ARGB or BGRA for some platforms
		if (colorType == PNG_COLOR_TYPE_RGB_ALPHA)
		{
			pixelformat = EPF_RGBA8;
		}
		else
		{
			pixelformat = EPF_RGB8;
		}

		// Update the changes
		png_get_IHDR(png_ptr,
			info_ptr,
			(png_uint_32*)&width,
			(png_uint_32*)&height,
			&bitDepth,
			&colorType,
			NULL,
			NULL,
			NULL);

		// Create the image structure to be filled by png data
		TImagePtr Image = ti_new TImage(pixelformat, width, height);

		// Create array of pointers to rows in image data
		png_bytepp rowPointers = ti_new png_bytep[height];
		if (!rowPointers)
		{
			_LOG(Error, "load png internal PNG create row pointers failure. [%s]\n",
				FileInput.GetFileName().c_str());
			png_destroy_read_struct(&png_ptr, NULL, NULL);
			return 0;
		}
		// Fill array of pointers to rows in image data
		uint8* data = Image->Lock();
		// flip png image
		//data				+= image->GetPitch() * (image->GetHeight() - 1);
		for (uint32 i = 0; i < height; ++i)
		{
			rowPointers[i] = data;
			data += Image->GetPitch();
		}

		// for proper error handling
		if (!setjmp(png_jmpbuf(png_ptr)))
		{
			// Read data using the library function that handles all transformations including interlacing
			png_read_image(png_ptr, rowPointers);
			png_read_end(png_ptr, NULL);
		}


		ti_delete[] rowPointers;
		Image->Unlock();
		png_destroy_read_struct(&png_ptr, &info_ptr, 0); // Clean up memory

		return Image;
	}

	bool TImage::SaveToPNG(const char* filename, int32 MipIndex)
	{
		FILE* fp;
		fp = fopen(filename, "wb");
		if (fp == NULL)
		{
			return false;
		}
		png_struct* png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		png_info* info_ptr = png_create_info_struct(png_ptr);

		const int32 Width = GetWidth();
		const int32 Height = GetHeight();
		uint8* Data = Lock();
		const int32 Pitch = GetPitch();

		png_init_io(png_ptr, fp);
		png_set_IHDR(png_ptr, info_ptr, Width, Height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
		png_write_info(png_ptr, info_ptr);
		png_bytep* row_pointers = new png_bytep[Height];
		for (int i = 0; i < Height; i++)
		{
			row_pointers[i] = (png_bytep)(Data + i * Pitch);
		}
		png_write_image(png_ptr, row_pointers);
		delete[] row_pointers;
		row_pointers = NULL;
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);

		fclose(fp);

		return true;
	}
}
