/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "ResHelper.h"
#include "ResTextureHelper.h"
#include "TImage.h"
#include "PlatformUtils.h"
#include "ispc_texcomp.h"

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
			_LOG(Error, "unknown texture pixel format.\n");
			TI_ASSERT(0);
		}

		int32 Faces = 1;
		if (TextureType == ETT_TEXTURE_CUBE)
			Faces = 6;

		Texture->Desc.Width = Width;
		Texture->Desc.Height = Height;
		Texture->Desc.AddressMode = ETC_REPEAT;
		Texture->Desc.SRGB = 0;
		Texture->Desc.Mips = Mips;
		Texture->ImageSurfaces.resize(Faces);
		
		const uint32 BlockSize = 4 * 4;

		TI_ASSERT(Faces * Mips == AstcBuffers.size());

		for (int32 f = 0; f < Faces; ++f)
		{
			Width = (ASTCHeader->xSize[2] << 16) + (ASTCHeader->xSize[1] << 8) + ASTCHeader->xSize[0];
			Height = (ASTCHeader->ySize[2] << 16) + (ASTCHeader->ySize[1] << 8) + ASTCHeader->ySize[0];

			TImage * Image = ti_new TImage(Texture->Desc.Format, Width, Height);
			Texture->ImageSurfaces[f] = Image;
			Image->AllocEmptyMipmaps();
			TI_ASSERT(00);

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
				TImage::TSurfaceData& Surface = Image->GetMipmap(Mips);
				Surface.W = W;
				Surface.H = H;
				TI_ASSERT(0); // Calc ASTC Row Pitch size;
				//Surface.BlockSize = BlockSize;
				Surface.Data.Put(SrcData, DataLength);

				Width /= 2;
				Height /= 2;
			}
		}

		return Texture;
	}

	TResTextureDefine* ConvertDdsToAstc(TResTextureDefine* DDSTexture, const TString& Filename, int32 LodBias, E_PIXEL_FORMAT TargetFormat)
	{
		if (TargetFormat != EPF_UNKNOWN)
		{
			if (TargetFormat == DDSTexture->Desc.Format)
			{
				return DDSTexture;
			}
			_LOG(Error, "Do not support convert to Target format yet.\n");
			TI_ASSERT(0);
		}

		if (DDSTexture->Desc.Format >= EPF_R16F && DDSTexture->Desc.Format <= EPF_RGBA32F)
		{
			// HDR save as dds directly
			return DDSTexture;
		}

		TImage * DecodeResult = DecodeDXT(DDSTexture);
		TI_ASSERT(DDSTexture->Desc.Type == ETT_TEXTURE_2D);

		if (DecodeResult == nullptr)
		{
			_LOG(Error, "Failed to decode dds to tga. [%s]\n", Filename.c_str());
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
		_LOG(Log, "Converting [%s] to ASTC:\n  %s\n", Filename.c_str(), ASTCConverter.c_str());

		TVector<TStreamPtr> AstcFileBuffers;
		int32 Faces = 1;
		TI_ASSERT(Faces * DDSTexture->Desc.Mips == DecodeResult->GetMipmapCount());

		for (int32 f = 0; f < Faces; ++f)
		{
			int32 W = DecodeResult->GetWidth();
			int32 H = DecodeResult->GetHeight();

			for (uint32 mip = 0; mip < DDSTexture->Desc.Mips; ++mip)
			{
				DecodeResult->SaveToTga(TempTGAName.c_str(), mip);

				// Convert to astc
				int ret = system(ASTCConverter.c_str());

				if (ret == 0)
				{
					TFile f;
					if (!f.Open(TempASTCName, EFA_READ))
					{
						_LOG(Error, "failed to read converted astc file.\n");
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
					_LOG(Error, "failed to convert astc file.\n");
				}

				W /= 2;
				H /= 2;
			}
		}

		TString Name, Path;
		GetPathAndName(Filename, Name, Path);

		TResTextureDefine* Texture = CreateTextureFromASTC(DDSTexture->Desc.Type, DDSTexture->Desc.Mips, AstcFileBuffers);
		Texture->Name = Name;
		Texture->Path = Path;

		DeleteTempFile(TempTGAName);
		DeleteTempFile(TempASTCName);

		return Texture;
	}

	TResTextureDefine* LoadTgaToAstc(const TResTextureSourceInfo& SrcInfo)
	{
		TVector<TImage*> Images;
		// Load Tga image and generate mipmap
		TI_ASSERT(0);
		if (Images.size() == 0)
		{
			_LOG(Error, "Failed to decode dds to tga. [%s]\n", SrcInfo.TextureSource.c_str());
			return nullptr;
		}
		return nullptr;

		//TImage* TGAImage = Images[0];
		//if (TargetFormat != EPF_UNKNOWN)
		//{
		//	if (TargetFormat == TGAImage->GetFormat())
		//	{
		//		return TGATexture;
		//	}
		//	_LOG(Error, "Do not support convert to Target format yet.\n");
		//	TI_ASSERT(0);
		//}

		// Find ASTC converter and do convert for LDR image
		//TString ExePath = GetExecutablePath();
		//TString ASTCConverter = ExePath + "/astcenc -c ";
		//const TString TempTGAName = "Temp.tga";
		//const TString TempASTCName = "Temp.astc";
		//const TString ConvertParam = " 6x6 -medium -silentmode";
		//ASTCConverter += TempTGAName + " ";
		//ASTCConverter += TempASTCName + ConvertParam;
		//_LOG(Log, "Converting : [%s] to ASTC.\n", Filename.c_str());

		//TVector<TStreamPtr> AstcFileBuffers;
		//int32 Faces = 1;
		// TGA do not support Cubemap
		//if (DDSTexture->Desc.Type == ETT_TEXTURE_CUBE)
		//	Faces = 6;
		
		//for (int32 f = 0; f < Faces; ++f)
		//{
		//	int32 W = Images[f * DDSTexture->Desc.Mips + 0]->GetWidth();
		//	int32 H = Images[f * DDSTexture->Desc.Mips + 0]->GetHeight();

		//	for (uint32 mip = 0; mip < DDSTexture->Desc.Mips; ++mip)
		//	{
		//		TImage* Image = Images[f * DDSTexture->Desc.Mips + mip];
		//		TI_ASSERT(Image->GetWidth() == W && Image->GetHeight() == H);
		//		Image->SaveToTga(TempTGAName.c_str());

		//		// Convert to astc
		//		int ret = system(ASTCConverter.c_str());

		//		if (ret == 0)
		//		{
		//			TFile f;
		//			if (!f.Open(TempASTCName, EFA_READ))
		//			{
		//				_LOG(Error, "failed to read converted astc file.\n");
		//				break;
		//			}

		//			TStreamPtr FileStream = ti_new TStream;

		//			// Load file to memory
		//			const int32 FileSize = f.GetSize();
		//			uint8* FileBuffer = ti_new uint8[FileSize];
		//			f.Read(FileBuffer, FileSize, FileSize);
		//			f.Close();

		//			FileStream->Put(FileBuffer, FileSize);
		//			ti_delete[] FileBuffer;
		//			AstcFileBuffers.push_back(FileStream);
		//		}
		//		else
		//		{
		//			_LOG(Error, "failed to convert astc file.\n");
		//		}

		//		W /= 2;
		//		H /= 2;
		//	}
		//}

		//TString Name, Path;
		//GetPathAndName(Filename, Name, Path);

		//TResTextureDefine* Texture = CreateTextureFromASTC(DDSTexture->Desc.Type, DDSTexture->Desc.Mips, AstcFileBuffers);
		//Texture->Name = Name;
		//Texture->Path = Path;

		//DeleteTempFile(TempTGAName);
		//DeleteTempFile(TempASTCName);

		//return Texture;
	}

	TResTextureDefine* TResTextureHelper::ConvertToAstc(TResTextureDefine* SrcImage)
	{
		E_PIXEL_FORMAT SrcFormat = SrcImage->ImageSurfaces[0]->GetFormat();
		E_PIXEL_FORMAT DstFormat = EPF_UNKNOWN;
		int32 BlockSize = 4;

		switch (TResSettings::GlobalSettings.AstcQuality)
		{
		case TResSettings::Astc_Quality_High:
			DstFormat = EPF_ASTC4x4;
			BlockSize = 4;
			break;
		case TResSettings::Astc_Quality_Mid:
			DstFormat = EPF_ASTC6x6;
			BlockSize = 6;
			break;
		default:
			DstFormat = EPF_ASTC8x8;
			BlockSize = 8;
			break;
		}

		bool bHasAlpha = false;
		if (SrcFormat == EPF_RGB8)
		{
			_LOG(Error, "Tga will never be RGB8 format.\n");
			TI_ASSERT(0);
			DstFormat = EPF_ASTC8x8;
		}
		else if (SrcFormat == EPF_RGBA8)
		{
			if (SrcImage->TGASourcePixelDepth == 24 && !TResSettings::GlobalSettings.ForceAlphaChannel)
			{
				bHasAlpha = false;
			}
			else
			{
				bHasAlpha = true;
			}
		}
		else
		{
			// Do not need compress
			return SrcImage;
		}

		astc_enc_settings AstcEncSetting;
		if (bHasAlpha)
		{
			GetProfile_astc_alpha_fast(&AstcEncSetting, BlockSize, BlockSize);
		}
		else
		{
			GetProfile_astc_fast(&AstcEncSetting, BlockSize, BlockSize);
		}

		TResTextureDefine* DstImage = ti_new TResTextureDefine;
		DstImage->Name = SrcImage->Name;
		DstImage->Path = SrcImage->Path;
		DstImage->LodBias = SrcImage->LodBias;
		DstImage->Desc = SrcImage->Desc;
		DstImage->Desc.Format = DstFormat;

		DstImage->ImageSurfaces.resize(SrcImage->ImageSurfaces.size());

		for (int32 i = 0; i < (int32)SrcImage->ImageSurfaces.size(); ++i)
		{
			TImage * TgaImage = SrcImage->ImageSurfaces[i];
			TImage * DxtImage = ti_new TImage(DstFormat, TgaImage->GetWidth(), TgaImage->GetHeight());
			DstImage->ImageSurfaces[i] = DxtImage;
			if (TgaImage->GetMipmapCount() > 1)
			{
				DxtImage->AllocEmptyMipmaps();
			}

			for (int32 Mip = 0; Mip < DxtImage->GetMipmapCount(); ++Mip)
			{
				const TImage::TSurfaceData& SrcData = TgaImage->GetMipmap(Mip);
				TImage::TSurfaceData& DstData = DxtImage->GetMipmap(Mip);

				rgba_surface Surface;
				Surface.ptr = (uint8_t*)SrcData.Data.GetBuffer();
				Surface.width = SrcData.W;
				Surface.height = SrcData.H;
				Surface.stride = SrcData.RowPitch;
                
                if (Surface.width < BlockSize || Surface.height < BlockSize)
                {
                    break;
                }

				uint8_t* Dst = (uint8_t*)DstData.Data.GetBuffer();
				// Call ISPC function to convert
				CompressBlocksASTC(&Surface, Dst, &AstcEncSetting);
			}
		}
		return DstImage;
	}
}
