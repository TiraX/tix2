/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "ResHelper.h"
#include "ResTextureHelper.h"
#include "TImage.h"
#if defined (TI_PLATFORM_IOS)
#include <mach-o/dyld.h>
#include <unistd.h>
#endif

namespace tix
{
	TString GetExecutablePath()
	{
		TString Ret;
		int8 Path[512];
#if defined (TI_PLATFORM_WIN32)
		::GetModuleFileName(NULL, Path, 512);
        Ret = Path;
        TStringReplace(Ret, "\\", "/");
#elif defined (TI_PLATFORM_IOS)
		uint32 BufferSize = 512;
		_NSGetExecutablePath(Path, &BufferSize);
		TI_ASSERT(BufferSize <= 512);
        Ret = Path;
        if (Ret.find("/Binary") == TString::npos && Ret.find("tix2/") == TString::npos)
        {
            Ret = getcwd(NULL, 0);
        }
#endif

		Ret = Ret.substr(0, Ret.rfind('/'));

		// if dir is not in Binary/ then find from root "tix2"
		if (Ret.find("/Binary") == TString::npos)
		{
			TString::size_type root_pos = Ret.find("tix2/");
			TI_ASSERT(root_pos != TString::npos);
			Ret = Ret.substr(0, root_pos + 5) + "Binary/";
#if defined (TI_PLATFORM_WIN32)
			Ret += "Windows";
#elif defined (TI_PLATFORM_IOS)
			Ret += "Mac";
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

	static TResTextureDefine* CreateTextureFromASTC(E_TEXTURE_TYPE TextureType, int32 Mips, const TVector<TStreamPtr>& AstcBuffers)
	{
		if (AstcBuffers.size() == 0)
			return nullptr;

		TStreamPtr BaseMipmap = AstcBuffers[0];

		TASTCHeader *ASTCHeader = (TASTCHeader *)(BaseMipmap->GetBuffer());

		if (ASTCHeader->magic != ASTCMagic)
		{
			return nullptr;
		}

		uint32 Width = (ASTCHeader->xSize[2] << 16) + (ASTCHeader->xSize[1] << 8) + ASTCHeader->xSize[0];
		uint32 Height = (ASTCHeader->ySize[2] << 16) + (ASTCHeader->ySize[1] << 8) + ASTCHeader->ySize[0];
		//uint32 Depth = (ASTCHeader->zSize[2] << 16) + (ASTCHeader->zSize[1] << 8) + ASTCHeader->zSize[0];

		int32 BlockDimX = ASTCHeader->blockDimX;
		int32 BlockDimY = ASTCHeader->blockDimY;
		//int32 BlockDimZ = ASTCHeader->blockDimZ;

		// Create the texture
		TResTextureDefine* Texture = ti_new TResTextureDefine();
		Texture->Desc.Type = TextureType;
		Texture->Desc.Format = GetASTCFormatByDimension(BlockDimX, BlockDimY, false);
		if (Texture->Desc.Format == EPF_UNKNOWN)
		{
			printf("Error: unknown texture pixel format.\n");
			TI_ASSERT(0);
		}
		Texture->Desc.Width = Width;
		Texture->Desc.Height = Height;
		Texture->Desc.AddressMode = ETC_REPEAT;
		Texture->Desc.SRGB = 0;
		Texture->Desc.Mips = Mips;
		Texture->Surfaces.resize((int32)AstcBuffers.size());
		
		const uint32 BlockSize = 4 * 4;

		int32 Faces = 1;
		if (TextureType == ETT_TEXTURE_CUBE)
			Faces = 6;

		TI_ASSERT(Faces * Mips == AstcBuffers.size());

		for (int32 f = 0; f < Faces; ++f)
		{
			Width = (ASTCHeader->xSize[2] << 16) + (ASTCHeader->xSize[1] << 8) + ASTCHeader->xSize[0];
			Height = (ASTCHeader->ySize[2] << 16) + (ASTCHeader->ySize[1] << 8) + ASTCHeader->ySize[0];

			for (int32 mip = 0; mip < Mips; ++mip)
			{
				TStreamPtr FileBuffer = AstcBuffers[f * Mips + mip];
				TASTCHeader* Header = (TASTCHeader*)FileBuffer->GetBuffer();

				uint32 W = (Header->xSize[2] << 16) + (Header->xSize[1] << 8) + Header->xSize[0];
				uint32 H = (Header->ySize[2] << 16) + (Header->ySize[1] << 8) + Header->ySize[0];
				uint32 D = (Header->zSize[2] << 16) + (Header->zSize[1] << 8) + Header->zSize[0];

				uint32 WidthInBlocks = (W + Header->blockDimX - 1) / Header->blockDimX;
				uint32 HeightInBlocks = (H + Header->blockDimY - 1) / Header->blockDimY;
				uint32 DepthInBlocks = (D + Header->blockDimZ - 1) / Header->blockDimZ;

				uint32 DataLength = WidthInBlocks * HeightInBlocks * DepthInBlocks * BlockSize;

				TI_ASSERT(W == Width && H == Height);

				const uint8* SrcData = (const uint8*)FileBuffer->GetBuffer() + sizeof(TASTCHeader);
				TResSurfaceData& Surface = Texture->Surfaces[mip];
				Surface.W = W;
				Surface.H = H;
				Surface.RowPitch = WidthInBlocks * BlockSize;
				Surface.Data.Put(SrcData, DataLength);

				Width /= 2;
				Height /= 2;
			}
		}

		return Texture;
	}

	TResTextureDefine* TResTextureHelper::LoadAstcFile(const TString& Filename, int32 LodBias)
	{
		TVector<TImage*> Images;

		TResTextureDefine* DDSTexture = nullptr;
		// if src format is dds, decode it first
		if (Filename.rfind(".dds") != TString::npos)
		{
			DDSTexture = TResTextureHelper::LoadDdsFile(Filename, LodBias);

			DecodeDXT(DDSTexture, Images);
		}
		else
		{
			printf("Error: unknown texture format. [%s]\n", Filename.c_str());
			return nullptr;
		}

		if (DDSTexture->Desc.Format >= EPF_R16F && DDSTexture->Desc.Format <= EPF_RGBA32F)
		{
			// HDR save as dds directly
			return DDSTexture;
		}

		if (Images.size() == 0)
		{
			printf("Error: Failed to decode dds to tga. [%s]\n", Filename.c_str());
			return nullptr;
		}
		// Find ASTC converter and do convert for LDR image
		TString ExePath = GetExecutablePath();
		TString ASTCConverter = ExePath + "/astcenc -c ";
		const TString& TempTGAName = "Temp.tga";
		const TString& TempASTCName = "Temp.astc";
		const TString ConvertParam = " 6x6 -medium -silentmode";
		ASTCConverter += TempTGAName + " ";
		ASTCConverter += TempASTCName + ConvertParam;
		printf("Converting [%s] to ASTC:\n  %s\n", Filename.c_str(), ASTCConverter.c_str());

		TVector<TStreamPtr> AstcFileBuffers;
		int32 Faces = 1;
		if (DDSTexture->Desc.Type == ETT_TEXTURE_CUBE)
			Faces = 6;

		TI_ASSERT(Faces * DDSTexture->Desc.Mips == Images.size());

		for (int32 f = 0; f < Faces; ++f)
		{
			int32 W = Images[f * DDSTexture->Desc.Mips + 0]->GetWidth();
			int32 H = Images[f * DDSTexture->Desc.Mips + 0]->GetHeight();

			for (uint32 mip = 0; mip < DDSTexture->Desc.Mips; ++mip)
			{
				TImage* Image = Images[f * DDSTexture->Desc.Mips + mip];
				TI_ASSERT(Image->GetWidth() == W && Image->GetHeight() == H);
				Image->SaveToTga(TempTGAName.c_str());

				// Convert to astc
				int ret = system(ASTCConverter.c_str());

				if (ret == 0)
				{
					TFile f;
					if (!f.Open(TempASTCName, EFA_READ))
					{
						printf("Error: failed to read converted astc file.\n");
						break;
					}

					TStreamPtr FileStream = ti_new TStream;

					// Load file to memory
					const int32 FileSize = f.GetSize();
					uint8* FileBuffer = ti_new uint8[FileSize];
					f.Read(FileBuffer, FileSize, FileSize);
					f.Close();

					FileStream->Put(FileBuffer, FileSize);
					ti_delete[] FileBuffer;
					AstcFileBuffers.push_back(FileStream);
				}
				else
				{
					printf("Error: failed to convert astc file.\n");
				}

				W /= 2;
				H /= 2;
			}
		}

		TString Name, Path;
		size_t Mark = Filename.rfind('/');
		if (Mark == TString::npos)
		{
			Name = Filename;
		}
		else
		{
			Name = Filename.substr(Mark + 1);
			Path = Filename.substr(0, Mark);
		}

		TResTextureDefine* Texture = CreateTextureFromASTC(DDSTexture->Desc.Type, DDSTexture->Desc.Mips, AstcFileBuffers);
		Texture->Name = Name;
		Texture->Path = Path;

		DeleteTempFile(TempTGAName);
		DeleteTempFile(TempASTCName);

		ti_delete DDSTexture;

		return Texture;
	}
}
