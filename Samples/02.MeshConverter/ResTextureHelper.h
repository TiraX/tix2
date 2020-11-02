/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "TImage.h"

namespace tix
{
	struct TResTextureDefine
	{
		TString Name;
		TString Path;
		int32 LodBias;

		TTextureDesc Desc;
		TVector<TImage*> ImageSurfaces;

		// Extra Info
		int32 TGASourcePixelDepth;

		TResTextureDefine()
			: LodBias(0)
			, TGASourcePixelDepth(0)
		{}
	};

	struct TResTextureSourceInfo
	{
		TString TextureSource;
		int32 LodBias;
		E_PIXEL_FORMAT TargetFormat;
		E_TEXTURE_ADDRESS_MODE AddressMode;
		int32 SRGB;
		int32 IsNormalmap;
		int32 IsIBL;
		int32 HasMips;

		TResTextureSourceInfo()
			: LodBias(0)
			, TargetFormat(EPF_RGBA8)
			, AddressMode(ETC_REPEAT)
			, SRGB(0)
			, IsNormalmap(0)
			, IsIBL(0)
			, HasMips(0)
		{}
	};

	class TImage;
	class TResTextureHelper
	{
	public:
		TResTextureHelper();
		~TResTextureHelper();

		static bool LoadTextureFile(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings);
		static TResTextureDefine* LoadDdsFile(const TResTextureSourceInfo& SrcInfo);
		static TResTextureDefine* LoadTgaFile(const TResTextureSourceInfo& SrcInfo);
		static TResTextureDefine* LoadHdrFile(const TResTextureSourceInfo& SrcInfo);

		static TResTextureDefine* ConvertToDds(TResTextureDefine* SrcImage);
		static TResTextureDefine* ConvertToAstc(TResTextureDefine* SrcImage);
		static TResTextureDefine* Convert32FTo16F(TResTextureDefine* SrcImage);

		static TResTextureDefine* LongLatToCubeAndFilter(TResTextureDefine* SrcImage);

		//static TResTextureDefine* ConvertDdsToAstc(TResTextureDefine* DdsTexture, const TString& Filename, int32 LodBias, E_PIXEL_FORMAT TargetFormat);

		//static TResTextureDefine* LoadTgaToDds(const TResTextureSourceInfo& SrcInfo);
		//static TResTextureDefine* LoadTgaToAstc(const TResTextureSourceInfo& SrcInfo);

		void AddTexture(TResTextureDefine* Texture);
		void OutputTexture(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
		TVector<TResTextureDefine*> Textures;
	};

	TImage* DecodeDXT(TResTextureDefine* Texture);
}
