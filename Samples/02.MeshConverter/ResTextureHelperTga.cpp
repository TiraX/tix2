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

	class TGenerateMipmapTask : public TResMTTask
	{
	public:
		TGenerateMipmapTask(TImage* InSrcImage, int32 TargetMip, int32 InYStart, int32 InYEnd)
			: SrcImage(InSrcImage)
			, Miplevel(TargetMip)
			, YStart(InYStart)
			, YEnd(InYEnd)
		{}
		TImage* SrcImage;
		int32 Miplevel;
		int32 YStart, YEnd;

		virtual void Exec() override
		{
			TI_ASSERT(Miplevel >= 1);
			TI_ASSERT(SrcImage->GetMipmapCount() > 1);
			int32 SrcMipLevel = Miplevel - 1;
			int32 W = SrcImage->GetMipmap(SrcMipLevel).W;
			//int32 H = SrcImage->GetMipmap(SrcMipLevel).H;

			// Down sample to generate mips

			for (int32 y = YStart * 2; y < YEnd * 2; y += 2)
			{
				for (int32 x = 0; x < W; x += 2)
				{
					SColor c00 = SrcImage->GetPixel(x + 0, y + 0, SrcMipLevel);
					SColor c10 = SrcImage->GetPixel(x + 1, y + 0, SrcMipLevel);
					SColor c01 = SrcImage->GetPixel(x + 0, y + 1, SrcMipLevel);
					SColor c11 = SrcImage->GetPixel(x + 1, y + 1, SrcMipLevel);

					float Rf, Gf, Bf, Af;
					Rf = (float)(c00.R + c10.R + c01.R + c11.R);
					Gf = (float)(c00.G + c10.G + c01.G + c11.G);
					Bf = (float)(c00.B + c10.B + c01.B + c11.B);
					Af = (float)(c00.A + c10.A + c01.A + c11.A);

					// Calc average
					SColor Target;
					Target.R = TMath::Round(Rf * 0.25f);
					Target.G = TMath::Round(Gf * 0.25f);
					Target.B = TMath::Round(Bf * 0.25f);
					Target.A = TMath::Round(Af * 0.25f);

					SrcImage->SetPixel(x / 2, y / 2, Target, Miplevel);
				}
			}
		}
	};

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
			TVector<TGenerateMipmapTask*> Tasks;
			Tasks.reserve(MaxThreads * TgaImage->GetMipmapCount());

			// Parallel for big mips
			for (int32 Mip = 1; Mip < TgaImage->GetMipmapCount(); ++Mip)
			{
				int32 H = TgaImage->GetMipmap(Mip).H;
				for (int32 y = 0; y < H; ++y)
				{
					TGenerateMipmapTask* Task = ti_new TGenerateMipmapTask(TgaImage.get(), Mip, y, y + 1);
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
