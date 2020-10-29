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
		TImagePtr HdrImage = TImage::LoadImageHDR(f);

		// Latlong to cubemap


		// Create the texture
		TResTextureDefine* Texture = ti_new TResTextureDefine();
		Texture->Desc.Type = ETT_TEXTURE_CUBE;
		Texture->Desc.Format = HdrImage->GetFormat();
		Texture->Desc.Width = HdrImage->GetWidth();
		Texture->Desc.Height = HdrImage->GetHeight();
		Texture->Desc.AddressMode = ETC_REPEAT;
		Texture->Desc.SRGB = SrcInfo.SRGB;
		Texture->Desc.Mips = HdrImage->GetMipmapCount();
		Texture->ImageSurfaces.resize(1);

		TString Name, Path;
		GetPathAndName(SrcPathName, Path, Name);
		Texture->Name = Name;
		Texture->Path = Path;

		TI_ASSERT(0);
		//Texture->ImageSurfaces[0] = HdrImage;
		return Texture;
	}
}
