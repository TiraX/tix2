/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "ResHelper.h"
#include "ResTextureHelper.h"
#include "TImage.h"
#include "ResMultiThreadTask.h"

namespace tix
{

	TResTextureDefine* TResTextureHelper::LoadHdrFile(const TResTextureSourceInfo& SrcInfo)
	{
		TFile f;
		TString SrcPathName = TResSettings::GlobalSettings.SrcPath + SrcInfo.TextureSource;
		if (!f.Open(SrcPathName, EFA_READ))
		{
			return nullptr;
		}
		TImagePtr HdrImage = TImage::LoadImageHDR(f, false);

		// Latlong to cubemap
		TVector<TImagePtr> FaceImages = HdrImage->LatlongToCube();

		// Do not Generate cube mips here. Since it need do PBR Filtering

		// do test
		//for (int32 Face = 0; Face < 6; ++Face)
		//{
		//	char name[128];
		//	sprintf(name, "test_cube_face_%d.hdr", Face);
		//	TImagePtr F = FaceImages[Face];
		//	F->SaveToHDR(name);
		//}

		TI_TODO("Do Mip Bias");

		// Create the texture
		TResTextureDefine* Texture = ti_new TResTextureDefine();
		Texture->Desc.Type = ETT_TEXTURE_CUBE;
		Texture->Desc.Format = FaceImages[0]->GetFormat();
		Texture->Desc.Width = FaceImages[0]->GetWidth();
		Texture->Desc.Height = FaceImages[0]->GetHeight();
		Texture->Desc.AddressMode = ETC_REPEAT;
		Texture->Desc.SRGB = SrcInfo.SRGB;
		Texture->Desc.Mips = FaceImages[0]->GetMipmapCount();
		Texture->ImageSurfaces.resize(6);
		for (int32 Face = 0; Face < 6; ++Face)
		{
			Texture->ImageSurfaces[Face] = ti_new TImage(FaceImages[Face]->GetFormat(), FaceImages[Face]->GetWidth(), FaceImages[Face]->GetHeight());
			const TImage::TSurfaceData& SrcMipData = FaceImages[Face]->GetMipmap(0);
			TImage::TSurfaceData& DestMipData = Texture->ImageSurfaces[Face]->GetMipmap(0);
			memcpy(DestMipData.Data.GetBuffer(), SrcMipData.Data.GetBuffer(), SrcMipData.Data.GetLength());
		}

		TString Name, Path;
		GetPathAndName(SrcPathName, Path, Name);
		Texture->Name = Name;
		Texture->Path = Path;

		return Texture;
	}

	TResTextureDefine* TResTextureHelper::Convert32FTo16F(TResTextureDefine* SrcImage)
	{
		E_PIXEL_FORMAT SrcFormat = SrcImage->ImageSurfaces[0]->GetFormat();
		TI_ASSERT(SrcFormat == EPF_RGBA32F);
		E_PIXEL_FORMAT DstFormat = EPF_RGBA16F;

		TResTextureDefine* DstImage = ti_new TResTextureDefine;
		DstImage->Name = SrcImage->Name;
		DstImage->Path = SrcImage->Path;
		DstImage->LodBias = SrcImage->LodBias;
		DstImage->Desc = SrcImage->Desc;
		DstImage->Desc.Format = DstFormat;

		DstImage->ImageSurfaces.resize(SrcImage->ImageSurfaces.size());

		for (int32 i = 0; i < (int32)SrcImage->ImageSurfaces.size(); ++i)
		{
			TImage* Src = SrcImage->ImageSurfaces[i];
			TImage* Dst = ti_new TImage(DstFormat, Src->GetWidth(), Src->GetHeight());
			for (int32 y = 0; y < Src->GetHeight(); ++y)
			{
				for (int32 x = 0; x < Src->GetWidth(); ++x)
				{
					Dst->SetPixel(x, y, Src->GetPixelFloat(x, y));
				}
			}
			DstImage->ImageSurfaces[i] = Dst;
		}
		return DstImage;
	}
}
