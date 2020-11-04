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
	typedef TGenerateMipmapTask<1> THdrMipmapTask;
	TResTextureDefine* TResTextureHelper::LoadHdrFile(const TResTextureSourceInfo& SrcInfo)
	{
		TFile f;
		TString SrcPathName = TResSettings::GlobalSettings.SrcPath + SrcInfo.TextureSource;
		if (!f.Open(SrcPathName, EFA_READ))
		{
			return nullptr;
		}
		TImagePtr HdrImage = TImage::LoadImageHDR(f, false);
		if (SrcInfo.HasMips)
		{
			// Generate mipmaps 
			HdrImage->AllocEmptyMipmaps();

			const int32 MaxThreads = TResMTTaskExecuter::Get()->GetMaxThreadCount();
			TVector<THdrMipmapTask*> Tasks;
			Tasks.reserve(MaxThreads * HdrImage->GetMipmapCount());

			// Parallel for big mips
			for (int32 Mip = 1; Mip < HdrImage->GetMipmapCount(); ++Mip)
			{
				int32 H = HdrImage->GetMipmap(Mip).H;
				if (H >= MaxThreads)
				{
					for (int32 y = 0; y < H; ++y)
					{
						THdrMipmapTask* Task = ti_new THdrMipmapTask(HdrImage.get(), Mip, y, y + 1);
						TResMTTaskExecuter::Get()->AddTask(Task);
						Tasks.push_back(Task);
					}
					TResMTTaskExecuter::Get()->StartTasks();
					TResMTTaskExecuter::Get()->WaitUntilFinished();
				}
				else
				{
					THdrMipmapTask* Task = ti_new THdrMipmapTask(HdrImage.get(), Mip, 0, H);
					Task->Exec();
					Tasks.push_back(Task);
				}
			}

			static bool bDebugMips = false;
			if (bDebugMips)
			{
				for (int32 Mip = 0; Mip < HdrImage->GetMipmapCount(); ++Mip)
				{
					char Name[256];
					sprintf(Name, "%s_%d.hdr", SrcInfo.TextureSource.c_str(), Mip);
					HdrImage->SaveToHDR(Name, Mip);
				}
			}

			// delete Tasks
			for (auto& T : Tasks)
			{
				ti_delete T;
			}
			Tasks.clear();
		}

		// Latlong to cubemap
		//TVector<TImagePtr> FaceImages = HdrImage->LatlongToCube();

		// Create the texture
		TResTextureDefine* Texture = ti_new TResTextureDefine();
		Texture->Desc.Type = ETT_TEXTURE_2D;
		Texture->Desc.Format = HdrImage->GetFormat();
		Texture->Desc.Width = HdrImage->GetWidth();
		Texture->Desc.Height = HdrImage->GetHeight();
		Texture->Desc.AddressMode = ETC_REPEAT;
		Texture->Desc.SRGB = SrcInfo.SRGB;
		Texture->Desc.Mips = HdrImage->GetMipmapCount();
		Texture->ImageSurfaces.resize(1); 
		Texture->ImageSurfaces[0] = ti_new TImage(HdrImage->GetFormat(), HdrImage->GetWidth(), HdrImage->GetHeight());
		if (Texture->Desc.Mips > 1)
		{
			Texture->ImageSurfaces[0]->AllocEmptyMipmaps();
		}
		for (int32 Mip = 0; Mip < (int32)Texture->Desc.Mips; ++Mip)
		{
			const TImage::TSurfaceData& SrcMipData = HdrImage->GetMipmap(Mip);
			TImage::TSurfaceData& DestMipData = Texture->ImageSurfaces[0]->GetMipmap(Mip);
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
