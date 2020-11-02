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
		const TImage::TSurfaceData& SrcMipData = HdrImage->GetMipmap(0);
		TImage::TSurfaceData& DestMipData = Texture->ImageSurfaces[0]->GetMipmap(0);
		memcpy(DestMipData.Data.GetBuffer(), SrcMipData.Data.GetBuffer(), SrcMipData.Data.GetLength());

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

	///////////////////////////////////////////////////////////////////////

	// Longlat image to cubemap
	// From UE4 TextureCompressorModule.cpp
	// transform world space vector to a space relative to the face
	static vector3df TransformSideToWorldSpace(uint32 CubemapFace, const vector3df& InDirection)
	{
		float x = InDirection.X, y = InDirection.Y, z = InDirection.Z;

		vector3df Ret;

		// see http://msdn.microsoft.com/en-us/library/bb204881(v=vs.85).aspx
		switch (CubemapFace)
		{
		case 0: Ret = vector3df(+z, -y, -x); break;
		case 1: Ret = vector3df(-z, -y, +x); break;
		case 2: Ret = vector3df(+x, +z, +y); break;
		case 3: Ret = vector3df(+x, -z, -y); break;
		case 4: Ret = vector3df(+x, -y, +z); break;
		case 5: Ret = vector3df(-x, -y, -z); break;
		default:
			TI_ASSERT(0);
			break;
		}

		// this makes it with the Unreal way (z and y are flipped)
		return vector3df(Ret.X, Ret.Z, Ret.Y);
	}

	// transform vector relative to the face to world space
	static vector3df TransformWorldToSideSpace(uint32 CubemapFace, const vector3df& InDirection)
	{
		// undo Unreal way (z and y are flipped)
		float x = InDirection.X, y = InDirection.Z, z = InDirection.Y;

		vector3df Ret;

		// see http://msdn.microsoft.com/en-us/library/bb204881(v=vs.85).aspx
		switch (CubemapFace)
		{
		case 0: Ret = vector3df(-z, -y, +x); break;
		case 1: Ret = vector3df(+z, -y, -x); break;
		case 2: Ret = vector3df(+x, +z, +y); break;
		case 3: Ret = vector3df(+x, -z, -y); break;
		case 4: Ret = vector3df(+x, -y, +z); break;
		case 5: Ret = vector3df(-x, -y, -z); break;
		default:
			TI_ASSERT(0);
			break;
		}

		return Ret;
	}

	vector3df ComputeSSCubeDirectionAtTexelCenter(uint32 x, uint32 y, float InvSideExtent)
	{
		// center of the texels
		vector3df DirectionSS((x + 0.5f) * InvSideExtent * 2 - 1, (y + 0.5f) * InvSideExtent * 2 - 1, 1);
		DirectionSS.normalize();
		return DirectionSS;
	}

	static vector3df ComputeWSCubeDirectionAtTexelCenter(uint32 CubemapFace, uint32 x, uint32 y, float InvSideExtent)
	{
		vector3df DirectionSS = ComputeSSCubeDirectionAtTexelCenter(x, y, InvSideExtent);
		vector3df DirectionWS = TransformSideToWorldSpace(CubemapFace, DirectionSS);
		return DirectionWS;
	}
	static int32 ComputeLongLatCubemapExtents(int32 ImageWidth)
	{
		int32 Extents = 1 << TMath::FloorLog2(ImageWidth / 2);
		return TMath::Max(Extents, 32);
	}

	static vector2df DirectionToLongLat(const vector3df& NormalizedDirection, int32 LongLatWidth, int32 LongLatHeight)
	{
		// see http://gl.ict.usc.edu/Data/HighResProbes
		// latitude-longitude panoramic format = equirectangular mapping

		vector2df Result;

		Result.X = (1 + atan2(NormalizedDirection.X, -NormalizedDirection.Z) / PI) / 2 * LongLatWidth;
		Result.Y = acos(NormalizedDirection.Y) / PI * LongLatHeight;

		return Result;
	}

	/** Wraps X around W. */
	static inline void WrapTo(int32& X, int32 W)
	{
		X = X % W;

		if (X < 0)
		{
			X += W;
		}
	}

	SColorf GetPixelFloatWithXWrap(TImage* Image, float x, float y, int32 MipIndex)
	{
		const int32 W = Image->GetWidth() >> MipIndex;
		const int32 H = Image->GetHeight() >> MipIndex;

		int32 X0 = (int32)floor(x);
		int32 Y0 = (int32)floor(y);

		float FracX = x - X0;
		float FracY = y - Y0;

		int32 X1 = X0 + 1;
		int32 Y1 = Y0 + 1;

		WrapTo(X0, W);
		WrapTo(X1, W);
		Y0 = TMath::Max(0, Y0); Y0 = TMath::Min(Y0, H - 1);
		Y1 = TMath::Max(0, Y1); Y1 = TMath::Min(Y1, H - 1);

		SColorf CornerRGB00 = Image->GetPixelFloat(X0, Y0, MipIndex);
		SColorf CornerRGB10 = Image->GetPixelFloat(X1, Y0, MipIndex);
		SColorf CornerRGB01 = Image->GetPixelFloat(X0, Y1, MipIndex);
		SColorf CornerRGB11 = Image->GetPixelFloat(X1, Y1, MipIndex);

		SColorf CornerRGB0 = TMath::Lerp(CornerRGB00, CornerRGB10, FracX);
		SColorf CornerRGB1 = TMath::Lerp(CornerRGB01, CornerRGB11, FracX);

		return TMath::Lerp(CornerRGB0, CornerRGB1, FracY);
	}

	TVector<TImage*> LongLatToCube(TImage* LongLat)
	{
		TVector<TImage*> Surfaces;
		Surfaces.resize(6);

		const int32 Extent = ComputeLongLatCubemapExtents(LongLat->GetWidth());
		const float InvExtent = 1.0f / Extent;
		const int32 W = LongLat->GetWidth();
		const int32 H = LongLat->GetHeight();

		for (int32 Face = 0; Face < 6; ++Face)
		{
			Surfaces[Face] = ti_new TImage(LongLat->GetFormat(), Extent, Extent);
		}

		for (int32 Face = 0; Face < 6; ++Face)
		{
			TImage* FaceImage = Surfaces[Face];

			for (int32 y = 0; y < Extent; ++y)
			{
				for (int32 x = 0; x < Extent; ++x)
				{
					vector3df DirectionWS = ComputeWSCubeDirectionAtTexelCenter(Face, x, y, InvExtent);
					vector2df Coord = DirectionToLongLat(DirectionWS, W, H);

					FaceImage->SetPixel(x, y, GetPixelFloatWithXWrap(LongLat, Coord.X, Coord.Y, 0));
				}
			}
		}

		return Surfaces;
	}

	TResTextureDefine* TResTextureHelper::LongLatToCubeAndFilter(TResTextureDefine* SrcImage)
	{
		TI_ASSERT(SrcImage->Desc.Type == ETT_TEXTURE_2D);
		TI_ASSERT(SrcImage->Desc.Width == SrcImage->Desc.Height * 2);

		TImage* LongLatImage = SrcImage->ImageSurfaces[0];
		// To cube map and saved for mipmap1
		// Mip 0
		TVector<TImage*> FaceImages = LongLatToCube(LongLatImage);

		// Alloc mips
		for (int32 Face = 0; Face < 6; Face++)
		{
			FaceImages[Face]->AllocEmptyMipmaps();
		}

		// Create mips and filter
		const int32 TotalMips = FaceImages[0]->GetMipmapCount();
		for (int32 Mip = 1; Mip < TotalMips; ++Mip)
		{
			const int32 Extent = ComputeLongLatCubemapExtents(LongLatImage->GetWidth() >> Mip);
			const float InvExtent = 1.0f / Extent;
			const int32 W = LongLatImage->GetWidth() >> Mip;
			const int32 H = LongLatImage->GetHeight() >> Mip;

			for (int32 Face = 0; Face < 6; ++Face)
			{
				TImage* FaceImage = FaceImages[Face];

				for (int32 y = 0; y < Extent; ++y)
				{
					for (int32 x = 0; x < Extent; ++x)
					{
						vector3df DirectionWS = ComputeWSCubeDirectionAtTexelCenter(Face, x, y, InvExtent);
						vector2df Coord = DirectionToLongLat(DirectionWS, W, H);

						Do importance sampling and filter
						// UE4 ReflectionEnvironmentShaders.usf, FilterPS()

						//FaceImage->SetPixel(x, y, GetPixelFloatWithXWrap(LongLat, Coord.X, Coord.Y), Mip);
					}
				}
			}
		}
	}
}
