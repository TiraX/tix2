/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResTextureHelper.h"
#include "ResMultiThreadTask.h"

namespace tix
{
	TResTextureHelper::TResTextureHelper()
	{
	}

	TResTextureHelper::~TResTextureHelper()
	{
		for (auto T : Textures)
		{
			ti_delete T;
		}
		Textures.clear();
	}

	void TResTextureHelper::AddTexture(TResTextureDefine* Texture)
	{
		Textures.push_back(Texture);
	}

	bool TResTextureHelper::LoadTextureFile(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TString Name = Doc["name"].GetString();
		//int32 Version = Doc["version"].GetInt();

		TResTextureSourceInfo SrcInfo;
		SrcInfo.SRGB = Doc["srgb"].GetInt();
		SrcInfo.AddressMode = GetAddressMode(Doc["address_mode"].GetString());
		SrcInfo.TargetFormat = GetPixelFormat(Doc["target_format"].GetString());

		SrcInfo.LodBias = Doc["lod_bias"].GetInt();
		SrcInfo.IsNormalmap = Doc["is_normalmap"].GetInt();
		SrcInfo.HasMips = Doc["has_mips"].GetInt();
		SrcInfo.TextureSource = Doc["source"].GetString();

		TString ExtName = GetExtName(SrcInfo.TextureSource);
		// Load Texture By Name
		TResTextureDefine * SrcImage = nullptr;
		TString SrcImageType;
		if (ExtName == "dds")
		{
			SrcImage = TResTextureHelper::LoadDdsFile(SrcInfo);
			SrcImageType = "DDS";
		}
		else if (ExtName == "tga")
		{
			SrcImage = TResTextureHelper::LoadTgaFile(SrcInfo);
			SrcImageType = "TGA";
		}
		else
		{
			_LOG(Error, "Unknown texture format : %s\n", SrcInfo.TextureSource.c_str());
			return false;
		}

		TResTextureDefine* TextureOutput = nullptr;
#if defined (TI_PLATFORM_WIN32)
		// Win32 Platform need DDS texture
		if (SrcImageType == "DDS")
		{
			TextureOutput = SrcImage;
		}
		else
		{
			TextureOutput = TResTextureHelper::ConvertToDds(SrcImage);
			ti_delete SrcImage;
		}
#elif defined (TI_PLATFORM_IOS)
		// iOS Platform need ASTC texture
		if (SrcImageType == "DDS")
		{
			_LOG(Error, "DDS to ASTC not implemented yet.\n");
			return false;
		}
		else
		{
			TextureOutput = TResTextureHelper::ConvertToAstc(SrcImage);
			ti_delete SrcImage;
		}
#endif

		if (TextureOutput != nullptr)
		{
			TI_ASSERT(SrcInfo.LodBias < (int32)TextureOutput->Desc.Mips);

			TextureOutput->LodBias = SrcInfo.LodBias;
			TextureOutput->Desc.AddressMode = SrcInfo.AddressMode;
			TextureOutput->Desc.SRGB = SrcInfo.SRGB;

			TResTextureHelper Helper;
			Helper.AddTexture(TextureOutput);
			Helper.OutputTexture(OutStream, OutStrings);

			return true;
		}
		else
		{
			_LOG(Error, "Can not load texture : %s.\n", SrcInfo.TextureSource.c_str());
			return false;
		}
	}
	
	void TResTextureHelper::OutputTexture(TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResfileChunkHeader ChunkHeader;
		ChunkHeader.ID = TIRES_ID_CHUNK_TEXTURE;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_TEXTURE;
		ChunkHeader.ElementCount = (int32)Textures.size();

		TStream HeaderStream, DataStream(1024 * 8);
		for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		{
			TResTextureDefine* Define = Textures[t];

			int32 Faces = 1;
			if (Define->Desc.Type == ETT_TEXTURE_CUBE)
			{
				Faces = 6;
			}

			// init header
			THeaderTexture TextureHeader;
			memset(&TextureHeader, 0, sizeof(THeaderTexture));

			TextureHeader.StrId_Name = AddStringToList(OutStrings, Define->Name);
			TextureHeader.Format = Define->Desc.Format;
			TextureHeader.Width = Define->Desc.Width;
			TextureHeader.Height = Define->Desc.Height;
			TextureHeader.Type = Define->Desc.Type;
			TextureHeader.AddressMode = Define->Desc.AddressMode;
			TextureHeader.SRGB = Define->Desc.SRGB;
			TextureHeader.Mips = Define->Desc.Mips;
			TextureHeader.Surfaces = (uint32)Define->ImageSurfaces.size() * Define->Desc.Mips;
			TI_ASSERT(Define->Desc.Mips == Define->ImageSurfaces[0]->GetMipmapCount());

			HeaderStream.Put(&TextureHeader, sizeof(THeaderTexture));
			FillZero4(HeaderStream);

			const TVector<TImage*>& Surfaces = Define->ImageSurfaces;
			for (int32 Face = 0; Face < Faces; ++Face)
			{
				for (uint32 Mip = 0; Mip < Define->Desc.Mips; ++Mip)
				{
					const TImage::TSurfaceData& Surface = Surfaces[Face]->GetMipmap(Mip);
					int32 DataLength = ti_align4(Surface.Data.GetLength());
					DataStream.Put(&Surface.W, sizeof(int32));
					DataStream.Put(&Surface.H, sizeof(int32));
					DataStream.Put(&Surface.RowPitch, sizeof(int32));
					DataStream.Put(&DataLength, sizeof(int32));

					DataStream.Put(Surface.Data.GetBuffer(), Surface.Data.GetLength());
					FillZero4(DataStream);
				}
			}
		}
		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}

	class TGenerateMipmapTask : public TResMTTask
	{
	public:
		TGenerateMipmapTask(TImage * InSrcImage, int32 TargetMip, int32 InYStart, int32 InYEnd)
			: SrcImage(InSrcImage)
			, Miplevel(TargetMip)
			, YStart(InYStart)
			, YEnd(InYEnd)
		{}
		TImage * SrcImage;
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
					Target.R = ti_round(Rf * 0.25f);
					Target.G = ti_round(Gf * 0.25f);
					Target.B = ti_round(Bf * 0.25f);
					Target.A = ti_round(Af * 0.25f);

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
		TImage * TgaImage = TImage::LoadImageTGA(f, &TgaPixelDepth);
		if (SrcInfo.HasMips)
		{
			// Generate mipmaps 
			TgaImage->AllocEmptyMipmaps();
			TVector<TGenerateMipmapTask*> Tasks;
			Tasks.reserve(MaxThreads * TgaImage->GetMipmapCount());
			
			// Parallel for big mips
			for (int32 Mip = 1 ; Mip < TgaImage->GetMipmapCount() ; ++ Mip)
			{
				int32 H = TgaImage->GetMipmap(Mip).H;
				if (H >= MaxThreads)
				{
					int32 Section = H / MaxThreads;
					for (int32 i = 0; i < MaxThreads; ++i)
					{
						TGenerateMipmapTask * Task = ti_new TGenerateMipmapTask(TgaImage, Mip, i * Section, (i + 1) * Section);
						TResMTTaskExecuter::Get()->AddTask(Task);
						Tasks.push_back(Task);
					}
				}
			}
			TResMTTaskExecuter::Get()->StartTasks();
			TResMTTaskExecuter::Get()->WaitUntilFinished();
			// Small mips generate here
			for (int32 Mip = 1; Mip < TgaImage->GetMipmapCount(); ++Mip)
			{
				int32 H = TgaImage->GetMipmap(Mip).H;
				if (H < MaxThreads)
				{
					TGenerateMipmapTask * Task = ti_new TGenerateMipmapTask(TgaImage, Mip, 0, H);
					Task->Exec();
					Tasks.push_back(Task);
				}
			}
			// delete Tasks
			for (auto& T : Tasks)
			{
				ti_delete T;
			}
			Tasks.clear();
		}

		TImage * TgaImageWithBias = TgaImage;
		if (SrcInfo.LodBias > 0)
		{
			TgaImageWithBias = ti_new TImage(TgaImage->GetFormat(), TgaImage->GetWidth() >> SrcInfo.LodBias, TgaImage->GetHeight() >> SrcInfo.LodBias);
			TgaImageWithBias->AllocEmptyMipmaps();
			for (int32 Mip = SrcInfo.LodBias ; Mip < TgaImage->GetMipmapCount(); ++ Mip)
			{
				const TImage::TSurfaceData& SrcMipData = TgaImage->GetMipmap(Mip);
				TImage::TSurfaceData& DestMipData = TgaImageWithBias->GetMipmap(Mip - SrcInfo.LodBias);

				TI_ASSERT(DestMipData.Data.GetBufferSize() == SrcMipData.Data.GetLength());
				memcpy(DestMipData.Data.GetBuffer(), SrcMipData.Data.GetBuffer(), SrcMipData.Data.GetLength());
			}
			ti_delete TgaImage;
		}

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
