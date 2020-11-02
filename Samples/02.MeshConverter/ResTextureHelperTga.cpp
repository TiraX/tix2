/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "ResHelper.h"
#include "ResTextureHelper.h"
#include "TImage.h"
#include "ResMultiThreadTask.h"
#include "ResTextureTaskHelper.h"

namespace tix
{
	TResTextureDefine* TResTextureHelper::LoadTgaFile(const TResTextureSourceInfo& SrcInfo)
	{
		TFile f;
		TString SrcPathName = TResSettings::GlobalSettings.SrcPath + SrcInfo.TextureSource;
		if (!f.Open(SrcPathName, EFA_READ))
		{
			return nullptr;
		}
		const int32 MaxThreads = TResMTTaskExecuter::Get()->GetMaxThreadCount();
		int32 TgaPixelDepth;
		TImagePtr TgaImage = TImage::LoadImageTGA(f, &TgaPixelDepth);
		if (SrcInfo.HasMips)
		{
			// Generate mipmaps 
			TgaImage->AllocEmptyMipmaps();
			TVector<TGenerateMipmapTask<0>*> Tasks;
			Tasks.reserve(MaxThreads * TgaImage->GetMipmapCount());

			// Parallel for big mips
			for (int32 Mip = 1; Mip < TgaImage->GetMipmapCount(); ++Mip)
			{
				int32 H = TgaImage->GetMipmap(Mip).H;
				for (int32 y = 0; y < H; ++y)
				{
					TGenerateMipmapTask<0>* Task = ti_new TGenerateMipmapTask<0>(TgaImage.get(), Mip, y, y + 1);
					TResMTTaskExecuter::Get()->AddTask(Task);
					Tasks.push_back(Task);
				}
				TResMTTaskExecuter::Get()->StartTasks();
				TResMTTaskExecuter::Get()->WaitUntilFinished();
			}

			static bool bDebugMips = false;
			if (bDebugMips)
			{
				for (int32 Mip = 0; Mip < TgaImage->GetMipmapCount(); ++Mip)
				{
					char Name[256];
					sprintf(Name, "%s_%d.tga", SrcInfo.TextureSource.c_str(), Mip);
					TgaImage->SaveToTGA(Name, Mip);
				}
			}

			// delete Tasks
			for (auto& T : Tasks)
			{
				ti_delete T;
			}
			Tasks.clear();
		}

		TImage* TgaImageWithBias = ti_new TImage(TgaImage->GetFormat(), TgaImage->GetWidth() >> SrcInfo.LodBias, TgaImage->GetHeight() >> SrcInfo.LodBias);
		TgaImageWithBias->AllocEmptyMipmaps();
		for (int32 Mip = SrcInfo.LodBias; Mip < TgaImage->GetMipmapCount(); ++Mip)
		{
			const TImage::TSurfaceData& SrcMipData = TgaImage->GetMipmap(Mip);
			TImage::TSurfaceData& DestMipData = TgaImageWithBias->GetMipmap(Mip - SrcInfo.LodBias);

			TI_ASSERT(DestMipData.Data.GetBufferSize() == SrcMipData.Data.GetLength());
			memcpy(DestMipData.Data.GetBuffer(), SrcMipData.Data.GetBuffer(), SrcMipData.Data.GetLength());
		}
		TgaImage = nullptr;

		// Create the texture
		TResTextureDefine* Texture = ti_new TResTextureDefine();
		Texture->Desc.Type = ETT_TEXTURE_2D;
		Texture->Desc.Format = TgaImageWithBias->GetFormat();
		Texture->Desc.Width = TgaImageWithBias->GetWidth();
		Texture->Desc.Height = TgaImageWithBias->GetHeight();
		Texture->Desc.AddressMode = ETC_REPEAT;
		Texture->Desc.SRGB = SrcInfo.SRGB;
		Texture->Desc.Mips = TgaImageWithBias->GetMipmapCount();
		Texture->ImageSurfaces.resize(1);

		Texture->TGASourcePixelDepth = TgaPixelDepth;

		TString Name, Path;
		GetPathAndName(SrcPathName, Path, Name);
		Texture->Name = Name;
		Texture->Path = Path;

		Texture->ImageSurfaces[0] = TgaImageWithBias;
		return Texture;
	}
}
