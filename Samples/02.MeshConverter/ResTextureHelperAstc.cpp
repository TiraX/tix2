/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "ResHelper.h"
#include "ResTextureHelper.h"
#if defined (TI_PLATFORM_IOS)
#include <mach-o/dyld.h>
#endif

namespace tix
{
	TString GetExecutablePath()
	{
		TString Ret;
		int8 Path[512];
#if defined (TI_PLATFORM_WIN32)
		::GetModuleFileName(NULL, Path, 512);
#elif defined (TI_PLATFORM_IOS)
		uint32 BufferSize = 512;
		_NSGetExecutablePath(Path, &BufferSize);
		TI_ASSERT(BufferSize <= 512);
#endif
		Ret = Path;
		TStringReplace(Ret, "\\", "/");

		Ret = Ret.substr(0, Ret.rfind('/'));

		// if dir is not in Binary/ then find from root "tix2"
		if (Ret.find("/Binary") == TString::npos)
		{
			TString::size_type root_pos = Ret.find("tix2/");
			TI_ASSERT(root_pos != TString::npos);
			Ret = Ret.substr(0, root_pos + 5) + "Binary/";
#if defined (TI_PLATFORM_WIN32)
			Ret += "Windows/";
#elif defined (TI_PLATFORM_IOS)
			Ret += "Mac/";
#endif
		}

		return Ret;
	}

	void DeleteTempFile(const TString& FileName)
	{
		TString CommandLine;
#if defined (TI_PLATFORM_WIN32)
		CommandLine = "del ";
#elif defined (TI_PLATFORM_IOS)
		CommandLine = "rm ";
#endif
		CommandLine += FileName;
		system(CommandLine.c_str());
	}

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
	
	struct TASTCHeader
	{
		uint32 magic;
		uint8 blockDimX;
		uint8 blockDimY;
		uint8 blockDimZ;
		uint8 xSize[3];
		uint8 ySize[3];
		uint8 zSize[3];
	};

	// Default alignment
#if defined(_MSC_VER)
#	pragma pack( pop, packing )
#elif defined(__ARMCC_VERSION) //RVCT compiler
#	pragma pop
#elif defined(TI_PLATFORM_IOS) //RVCT compiler
#	pragma pop
#endif


	static const uint32 ASTCMagic = 0x5CA1AB13;

	E_PIXEL_FORMAT GetASTCFormatByDimension(uint32 BlockWidth, uint32 BlockHeight, bool bSRGB)
	{
		E_PIXEL_FORMAT Format = EPF_UNKNOWN;
		if (BlockWidth == 4 && BlockHeight == 4)
			Format = EPF_ASTC4x4;
		else if (BlockWidth == 6 && BlockHeight == 6)
			Format = EPF_ASTC6x6;
		else if (BlockWidth == 8 && BlockHeight == 8)
			Format = EPF_ASTC8x8;

		if (Format != EPF_UNKNOWN && bSRGB)
			Format = (E_PIXEL_FORMAT)(Format + 1);

		return Format;
	}

	static TResTextureDefine* CreateTextureFromASTC(
		const uint8* Data,
		uint32 DataSize)
	{
		TASTCHeader *ASTCHeader = (TASTCHeader *)(Data);

		if (ASTCHeader->magic != ASTCMagic)
		{
			return nullptr;
		}

		uint32 width = (ASTCHeader->xSize[2] << 16) + (ASTCHeader->xSize[1] << 8) + ASTCHeader->xSize[0];
		uint32 height = (ASTCHeader->ySize[2] << 16) + (ASTCHeader->ySize[1] << 8) + ASTCHeader->ySize[0];
		uint32 depth = (ASTCHeader->zSize[2] << 16) + (ASTCHeader->zSize[1] << 8) + ASTCHeader->zSize[0];

		uint32 widthInBlocks = (width + ASTCHeader->blockDimX - 1) / ASTCHeader->blockDimX;
		uint32 heightInBlocks = (height + ASTCHeader->blockDimY - 1) / ASTCHeader->blockDimY;
		uint32 depthInBlocks = (depth + ASTCHeader->blockDimZ - 1) / ASTCHeader->blockDimZ;

		uint32 blockSize = 4 * 4;
		uint32 dataLength = widthInBlocks * heightInBlocks * depthInBlocks * blockSize;

		{
			// Create the texture
			TResTextureDefine* Texture = ti_new TResTextureDefine();
			Texture->Desc.Type = ETT_TEXTURE_2D;

			if (Texture->Desc.Type == ETT_TEXTURE_UNKNOWN)
			{
				printf("Error: unknown texture type.\n");
				TI_ASSERT(0);
			}
			Texture->Desc.Format = GetASTCFormatByDimension(ASTCHeader->blockDimX, ASTCHeader->blockDimY, false);
			if (Texture->Desc.Format == EPF_UNKNOWN)
			{
				printf("Error: unknown texture pixel format.\n");
				TI_ASSERT(0);
			}
			Texture->Desc.Width = width;
			Texture->Desc.Height = height;
			Texture->Desc.AddressMode = ETC_REPEAT;
			Texture->Desc.SRGB = 0;
			Texture->Desc.Mips = 1;

			Texture->Surfaces.resize(1 * 1);

			TI_TODO("ASTC Mipmaps.");

			const uint8* SrcData = Data + sizeof(TASTCHeader);
			for (uint32 j = 0; j < 1; j++)
			{
				uint32 w = width;
				uint32 h = height;
				uint32 d = depth;
				for (uint32 i = 0; i < 1; i++)
				{
					TResSurfaceData& Surface = Texture->Surfaces[j * 1 + i];
					Surface.W = w;
					Surface.H = h;
					Surface.RowPitch = 0;
					Surface.Data.Put(SrcData, dataLength);

					SrcData += dataLength;

					w = w >> 1;
					h = h >> 1;
					d = d >> 1;
					if (w == 0)
					{
						w = 1;
					}
					if (h == 0)
					{
						h = 1;
					}
					if (d == 0)
					{
						d = 1;
					}
				}
			}
			return Texture;
		}
	}

	TResTextureDefine* TResTextureHelper::LoadAstcFile(const TString& Filename)
	{
		TString TextureName;
		TString TGAName;
		// if src format is dds, decode it first
		if (Filename.rfind(".dds") != TString::npos)
		{
			TGAName = Filename.substr(0, Filename.rfind(".dds")) + ".tga";
			if (!DecodeDXT(Filename, TGAName))
			{
				printf("Error: Failed to decode dds to tga. [%s]\n", Filename.c_str());
				return nullptr;
			}
			TextureName = TGAName;
		}
		else if (Filename.rfind(".tga") != TString::npos)
		{
			TextureName = Filename;
		}
		else
		{
			printf("Error: unknown texture format. [%s]\n", Filename.c_str());
			return nullptr;
		}

		TString ASTCName = TextureName.substr(0, TextureName.rfind('.')) + ".astc";

		// Find ASTC converter
		TString ExePath = GetExecutablePath();
		TString ASTCConverter = ExePath + "astcenc -c ";
		ASTCConverter += TextureName + " ";
		ASTCConverter += ASTCName + " 6x6 -medium -silentmode";
		printf("Converting ASTC: %s\n", ASTCConverter.c_str());

		// Convert to astc
		int ret = system(ASTCConverter.c_str());

        if (ret == 0)
		{
			TFile f;
			if (!f.Open(ASTCName, EFA_READ))
			{
				return nullptr;
			}

			// Load file to memory
			const int32 FileSize = f.GetSize();
			uint8* FileBuffer = ti_new uint8[FileSize];
			f.Read(FileBuffer, FileSize, FileSize);
			f.Close();

			TString Name, Path;
			size_t Mark = ASTCName.rfind('/');
			if (Mark == TString::npos)
			{
				Name = ASTCName;
			}
			else
			{
				Name = ASTCName.substr(Mark + 1);
				Path = ASTCName.substr(0, Mark);
			}

			TResTextureDefine* Texture = CreateTextureFromASTC(FileBuffer, FileSize);
			Texture->Name = Name;
			Texture->Path = Path;

			ti_delete[] FileBuffer;

			DeleteTempFile(TGAName);
			DeleteTempFile(ASTCName);
			return Texture;
        }
		return nullptr;
	}
}
