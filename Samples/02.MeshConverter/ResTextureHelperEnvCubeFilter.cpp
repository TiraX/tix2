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
		return DirectionSS;
	}

	static vector3df ComputeWSCubeDirectionAtTexelCenter(uint32 CubemapFace, uint32 x, uint32 y, float InvSideExtent)
	{
		vector3df DirectionSS = ComputeSSCubeDirectionAtTexelCenter(x, y, InvSideExtent);
		vector3df DirectionWS = TransformSideToWorldSpace(CubemapFace, DirectionSS);
		DirectionWS.normalize();
		return DirectionWS;
	}
	static int32 ComputeLongLatCubemapExtents(int32 ImageWidth)
	{
		int32 Extents = 1 << TMath::FloorLog2(ImageWidth / 2);
		TI_ASSERT(Extents > 0);
		return Extents;// TMath::Max(Extents, 32);
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

	inline vector3df LongLatToPosition(const vector2df& Coord)
	{
		float Lat = - (Coord.Y - 0.5f) * PI;
		// Long coord offset PI/2 to match unreal way.
		float Long = Coord.X * PI * 2.f + PI * 0.5f;

		vector3df Result;
		Result.X = cos(Lat) * cos(Long);
		Result.Y = cos(Lat) * sin(Long);
		Result.Z = sin(Lat);


		// fix this with unreal way.
		// this makes it with the Unreal way (z and y are flipped)
		return vector3df(Result.X, Result.Z, Result.Y);
	}

	SColorf SampleLongLat(TImage* LongLat, const vector3df& Dir, int32 Mip)
	{
		int32 W = LongLat->GetMipmap(Mip).W;
		int32 H = LongLat->GetMipmap(Mip).H;

		auto DirectionToLongLat = [](const vector3df& NormalizedDirection, int32 LongLatWidth, int32 LongLatHeight)
		{
			// see http://gl.ict.usc.edu/Data/HighResProbes
			// latitude-longitude panoramic format = equirectangular mapping

			vector2df Result;

			Result.X = (1 + atan2(NormalizedDirection.X, -NormalizedDirection.Z) / PI) / 2 * LongLatWidth;
			Result.Y = acos(NormalizedDirection.Y) / PI * LongLatHeight;

			return Result;
		};
		
		auto GetPixelFloatWithXWrap = [](TImage* Image, float x, float y, int32 MipIndex)
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
		};

		vector2df Coord = DirectionToLongLat(Dir, W, H);

		return GetPixelFloatWithXWrap(LongLat, Coord.X, Coord.Y, Mip);
	}

	SColorf SampleLongLatLinearMip(TImage* LongLat, const vector3df& Dir, float Mip)
	{
		int32 Mip0 = (int32)Mip;
		int32 Mip1 = Mip0 + 1;

		SColorf C0 = SampleLongLat(LongLat, Dir, Mip0);
		SColorf C1 = SampleLongLat(LongLat, Dir, Mip1);

		float Frac = Mip - Mip0;

		return TMath::Lerp(C0, C1, Frac);
	}

	TVector<TImage*> LongLatToCube(TImage* LongLat, bool WithMips)
	{
		TVector<TImage*> Surfaces;
		Surfaces.resize(6);

		int32 Extent = ComputeLongLatCubemapExtents(LongLat->GetWidth());
		float InvExtent = 1.0f / Extent;
		int32 W = LongLat->GetWidth();
		int32 H = LongLat->GetHeight();

		for (int32 Face = 0; Face < 6; ++Face)
		{
			Surfaces[Face] = ti_new TImage(LongLat->GetFormat(), Extent, Extent);

			if (WithMips && LongLat->GetMipmapCount() > 1)
			{
				Surfaces[Face]->AllocEmptyMipmaps();
			}
		}

		int32 TotalMips = WithMips ? LongLat->GetMipmapCount() : 1;

		for (int32 Mip = 0; Mip < TotalMips; Mip++)
		{
			W = LongLat->GetMipmap(Mip).W;
			H = LongLat->GetMipmap(Mip).H;
			Extent = ComputeLongLatCubemapExtents(W);
			InvExtent = 1.0f / Extent;

			for (int32 Face = 0; Face < 6; ++Face)
			{
				TImage* FaceImage = Surfaces[Face];

				for (int32 y = 0; y < Extent; ++y)
				{
					for (int32 x = 0; x < Extent; ++x)
					{
						vector3df DirectionWS = ComputeWSCubeDirectionAtTexelCenter(Face, x, y, InvExtent);
						SColorf Sample = SampleLongLat(LongLat, DirectionWS, Mip);

						FaceImage->SetPixel(x, y, Sample, Mip);
					}
				}
			}
		}

		return Surfaces;
	}

	// From UE4 Shaders
	inline float Pow2(float x)
	{
		return x * x;
	}
	inline float Pow4(float x)
	{
		float xx = x * x;
		return xx * xx;
	}
	inline matrix4 GetTangentBasis(const vector3df& TangentZ)
	{
		const float Sign = TangentZ.Z >= 0.f ? 1.f : -1.f;
		const float a = -1.f/(Sign + TangentZ.Z);
		const float b = TangentZ.X * TangentZ.Y * a;

		vector3df TangentX = vector3df( 1 + Sign * a * Pow2(TangentZ.X), Sign * b, -Sign * TangentZ.X );
		vector3df TangentY = vector3df( b,  Sign + a * Pow2(TangentZ.Y), -TangentZ.Y );

		matrix4 Result;
		Result[0] = TangentX.X;
		Result[1] = TangentX.Y;
		Result[2] = TangentX.Z;

		Result[4] = TangentY.X;
		Result[5] = TangentY.Y;
		Result[6] = TangentY.Z;

		Result[8] = TangentZ.X;
		Result[9] = TangentZ.Y;
		Result[10] = TangentZ.Z;

		return Result;// float3x3(TangentX, TangentY, TangentZ);
	}

#define REFLECTION_CAPTURE_ROUGHEST_MIP 1
#define REFLECTION_CAPTURE_ROUGHNESS_MIP_SCALE 1.2f

	/**
	 * Compute absolute mip for a reflection capture cubemap given a roughness.
	 */
	inline float ComputeReflectionCaptureMipFromRoughness(float Roughness, float CubemapMaxMip)
	{
		// Heuristic that maps roughness to mip level
		// This is done in a way such that a certain mip level will always have the same roughness, regardless of how many mips are in the texture
		// Using more mips in the cubemap just allows sharper reflections to be supported
		float LevelFrom1x1 = REFLECTION_CAPTURE_ROUGHEST_MIP - REFLECTION_CAPTURE_ROUGHNESS_MIP_SCALE * log2(TMath::Max(Roughness, 0.001f));
		return CubemapMaxMip - 1 - LevelFrom1x1;
	}

	inline float ComputeReflectionCaptureRoughnessFromMip(int32 Mip, int32 CubemapMaxMip)
	{
		float LevelFrom1x1 = float(CubemapMaxMip - 1 - Mip);
		return exp2((REFLECTION_CAPTURE_ROUGHEST_MIP - LevelFrom1x1) / REFLECTION_CAPTURE_ROUGHNESS_MIP_SCALE);
	}

	/** Reverses all the 32 bits. */
	inline uint32 ReverseBits32(uint32 bits)
	{
		bits = (bits << 16) | (bits >> 16);
		bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
		bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
		bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
		bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
		return bits;
	}
	
	inline vector2df Hammersley(uint32 Index, uint32 NumSamples, const vector2du& Random)
	{
		//float E1 = frac((float)Index / NumSamples + float(Random.X & 0xffff) / (1 << 16));
		float IntPart;
		float E1 = modf((float)Index / NumSamples + float(Random.X & 0xffff) / (1 << 16), &IntPart);
		float E2 = float(ReverseBits32(Index) ^ Random.Y) * 2.3283064365386963e-10f;
		return vector2df(E1, E2);
	}

	inline vector4df CosineSampleHemisphere(const vector2df& E)
	{
		float Phi = 2 * PI * E.X;
		float CosTheta = sqrt(E.Y);
		float SinTheta = sqrt(1 - CosTheta * CosTheta);

		vector3df H;
		H.X = SinTheta * cos(Phi);
		H.Y = SinTheta * sin(Phi);
		H.Z = CosTheta;

		float PDF = CosTheta * (1.0f / PI);

		return vector4df(H.X, H.Y, H.Z, PDF);
	}

	inline vector4df ImportanceSampleGGX(const vector2df& E, float a2)
	{
		float Phi = 2 * PI * E.X;
		float CosTheta = sqrt((1 - E.Y) / (1 + (a2 - 1) * E.Y));
		float SinTheta = sqrt(1 - CosTheta * CosTheta);

		vector3df H;
		H.X = SinTheta * cos(Phi);
		H.Y = SinTheta * sin(Phi);
		H.Z = CosTheta;

		float d = (CosTheta * a2 - CosTheta) * CosTheta + 1;
		float D = a2 / (PI * d * d);
		float PDF = D * CosTheta;

		return vector4df(H.X, H.Y, H.Z, PDF);
	}

	// GGX / Trowbridge-Reitz
	// [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
	inline float D_GGX(float a2, float NoH)
	{
		float d = (NoH * a2 - NoH) * NoH + 1;	// 2 mad
		return a2 / (PI * d * d);					// 4 mul, 1 rcp
	}

	class TEnvFilterTask : public TResMTTask
	{
	public:
		TEnvFilterTask(
			TImage* InSrcLongLat, 
			TImage* InFilteredLongLat,
			int32 InTargetMip, 
			int32 InTotalMips,
			float InRoughness, 
			float InSolidAngleTexel, 
			int32 InYStart, 
			int32 InYEnd
		)
			: SrcLongLat(InSrcLongLat)
			, FilteredLongLat(InFilteredLongLat)
			, TargetMip(InTargetMip)
			, TotalMips(InTotalMips)
			, Roughness(InRoughness)
			, SolidAngleTexel(InSolidAngleTexel)
			, YStart(InYStart)
			, YEnd(InYEnd)
		{}
		TImage* SrcLongLat;
		TImage* FilteredLongLat;
		int32 TargetMip;
		int32 TotalMips;
		float Roughness;
		float SolidAngleTexel;
		int32 YStart, YEnd;

		virtual void Exec() override
		{
			const uint32 NumSamples = Roughness < 0.1f ? 32 : 64;
			int32 W = SrcLongLat->GetWidth() >> TargetMip;
			int32 H = SrcLongLat->GetHeight() >> TargetMip;

			for (int32 y = YStart; y < YEnd; ++y)
			{
				for (int32 x = 0; x < W; ++x)
				{
					vector3df DirectionWS = LongLatToPosition(vector2df((x + 0.5f) / float(W), (y + 0.5f) / float(H)));
					matrix4 TangentToWorld = GetTangentBasis(DirectionWS);

					SColorf OutColor;

					if (Roughness < 0.01f)
					{
						OutColor = SrcLongLat->GetPixelFloat(x, y, TargetMip);
					}
					else
					{
						vector4df FilteredColor;
						if (Roughness > 0.99f)
						{
							// Roughness=1, GGX is constant. Use cosine distribution instead

							for (uint32 i = 0; i < NumSamples; i++)
							{
								vector2df E = Hammersley(i, NumSamples, vector2du());

								vector4df CosSample = CosineSampleHemisphere(E);
								vector3df L = vector3df(CosSample.X, CosSample.Y, CosSample.Z);

								float NoL = L.Z;

								float PDF = NoL / PI;
								float SolidAngleSample = 1.f / (NumSamples * PDF);
								float Mip = 0.5f * log2(SolidAngleSample / SolidAngleTexel);
								Mip = TMath::Clamp(Mip, 0.f, float(TotalMips - 1) - 0.01f);

								//L = mul(L, TangentToWorld);
								TangentToWorld.transformVect(L);
								L.normalize();
								vector4df Sample = SampleLongLatLinearMip(SrcLongLat, L, Mip);
								FilteredColor += Sample;
							}

							FilteredColor /= float(NumSamples);
						}
						else
						{
							float Weight = 0;

							for (uint32 i = 0; i < NumSamples; i++)
							{
								vector2df E = Hammersley(i, NumSamples, vector2du());

								// 6x6 Offset rows. Forms uniform star pattern
								//uint2 Index = uint2( i % 6, i / 6 );
								//float2 E = ( Index + 0.5 ) / 5.8;
								//E.x = frac( E.x + (Index.y & 1) * (0.5 / 6.0) );

								E.Y *= 0.995f;

								vector4df ImportanceSample = ImportanceSampleGGX(E, Pow4(Roughness));
								vector3df HalfVector = vector3df(ImportanceSample.X, ImportanceSample.Y, ImportanceSample.Z);
								vector3df L = 2 * HalfVector.Z * HalfVector - vector3df(0, 0, 1);

								float NoL = L.Z;
								float NoH = HalfVector.Z;

								if (NoL > 0.f)
								{
									float PDF = D_GGX(Pow4(Roughness), NoH) * 0.25f;
									float SolidAngleSample = 1.0f / (NumSamples * PDF);
									float Mip = 0.5f * log2(SolidAngleSample / SolidAngleTexel);
									Mip = TMath::Clamp(Mip, 0.f, float(TotalMips - 1) - 0.01f);

									float ConeAngle = acos(1 - SolidAngleSample / (2 * PI));

									//L = mul(L, TangentToWorld);
									TangentToWorld.transformVect(L);
									L.normalize();
									vector4df Sample = SampleLongLatLinearMip(SrcLongLat, L, Mip);
									FilteredColor += Sample * NoL;
									Weight += NoL;
								}
							}
							FilteredColor /= Weight;
						}
						OutColor.R = FilteredColor.X;
						OutColor.G = FilteredColor.Y;
						OutColor.B = FilteredColor.Z;
						OutColor.A = FilteredColor.W;
					}

					FilteredLongLat->SetPixel(x, y, OutColor, TargetMip);
				}
			}
		}
	};

	void ExportCubeMap(const TVector<TImage*>& FaceImages, const int8* NamePrefix)
	{
		TImage* Face0 = FaceImages[0];
		int32 W = Face0->GetWidth();
		int32 H = Face0->GetHeight();
		int32 PixelSize = TImage::GetPixelSizeInBytes(FaceImages[0]->GetFormat());

		TImage* Combined = ti_new TImage(FaceImages[0]->GetFormat(), W * 6, H);
		if (Face0->GetMipmapCount() > 1)
		{
			Combined->AllocEmptyMipmaps();
		}

		for (int32 Mip = 0; Mip < Face0->GetMipmapCount(); Mip++)
		{
			W = Face0->GetWidth() >> Mip;
			H = Face0->GetWidth() >> Mip;
			for (int32 i = 0; i < 6; i++)
			{
				int32 LineOffset = W * i;

				const TImage::TSurfaceData& SrcMipData = FaceImages[i]->GetMipmap(Mip);
				TImage::TSurfaceData& DestMipData = Combined->GetMipmap(Mip);
				int32 SrcPitch = SrcMipData.RowPitch;
				int32 DstPitch = DestMipData.RowPitch;
				for (int32 y = 0; y < H; y++)
				{
					memcpy(DestMipData.Data.GetBuffer() + y * DstPitch + LineOffset * PixelSize, SrcMipData.Data.GetBuffer() + y * SrcPitch, SrcPitch);
				}
			}

			char name[128];
			sprintf(name, "%s_mip%d.hdr", NamePrefix, Mip);
			Combined->SaveToHDR(name, Mip);
		}
		ti_delete Combined;
	}

	TResTextureDefine* TResTextureHelper::LongLatToCubeAndFilter(TResTextureDefine* SrcImage)
	{
		TIMER_RECORDER_FUNC();
		TI_ASSERT(SrcImage->Desc.Type == ETT_TEXTURE_2D);
		TI_ASSERT(SrcImage->Desc.Width == SrcImage->Desc.Height * 2);

		TImage* LongLatImage = SrcImage->ImageSurfaces[0];

		TImage* Filtered = ti_new TImage(LongLatImage->GetFormat(), LongLatImage->GetWidth(), LongLatImage->GetHeight());
		Filtered->AllocEmptyMipmaps();



		// Create mips and filter
		const int32 TotalMips = LongLatImage->GetMipmapCount();
		uint32 CubeSize = 1 << (TotalMips - 1);
		const float SolidAngleTexel = 4 * PI / (6 * CubeSize * CubeSize) * 2;

		for (int m = 0; m < TotalMips; ++m)
		{
			float Roughness = ComputeReflectionCaptureRoughnessFromMip(m, TotalMips - 1);
			_LOG(Log, "m = %d, R = %f.\n", m, Roughness);
		}

#define MULTI_THREAD 1
		const int32 MaxThreads = TResMTTaskExecuter::Get()->GetMaxThreadCount();
		TVector<TEnvFilterTask*> Tasks;
		Tasks.reserve(size_t(MaxThreads * TotalMips));

		for (int32 MipIndex = 0; MipIndex < TotalMips; ++MipIndex)
		{
			int32 W = LongLatImage->GetWidth() >> MipIndex;
			int32 H = LongLatImage->GetHeight() >> MipIndex;
			const int32 Extent = ComputeLongLatCubemapExtents(W);
			const float InvExtent = 1.0f / Extent;

			float Roughness = ComputeReflectionCaptureRoughnessFromMip(MipIndex, TotalMips - 1);
			if (H >= MaxThreads)
			{
				const int32 RowStep = H / MaxThreads;
				for (int32 y = 0; y < H; y += RowStep)
				{
					TEnvFilterTask* Task = ti_new TEnvFilterTask(
						LongLatImage,
						Filtered,
						MipIndex,
						TotalMips,
						Roughness,
						SolidAngleTexel,
						y,
						y + RowStep
					);
#if MULTI_THREAD
					TResMTTaskExecuter::Get()->AddTask(Task);
#else
					Task->Exec();
#endif
					Tasks.push_back(Task);
				}
			}
			else
			{
				TEnvFilterTask* Task = ti_new TEnvFilterTask(
					LongLatImage,
					Filtered,
					MipIndex,
					TotalMips,
					Roughness,
					SolidAngleTexel,
					0,
					H
				);
#if MULTI_THREAD
				TResMTTaskExecuter::Get()->AddTask(Task);
#else
				Task->Exec();
#endif
				Tasks.push_back(Task);
			}
		}

#if MULTI_THREAD
		TResMTTaskExecuter::Get()->StartTasks();
		TResMTTaskExecuter::Get()->WaitUntilFinished();
#endif
		// delete Tasks
		for (auto& T : Tasks)
		{
			ti_delete T;
		}
		Tasks.clear();

		TVector<TImage*> FaceImages = LongLatToCube(Filtered, true);

		// Debug
		if (false)
		{
			ExportCubeMap(FaceImages, "Filtered1");
		}
		ti_delete Filtered;

		// Create the texture
		TResTextureDefine* Texture = ti_new TResTextureDefine();
		Texture->Desc.Type = ETT_TEXTURE_CUBE;
		Texture->Desc.Format = LongLatImage->GetFormat();
		Texture->Desc.Width = FaceImages[0]->GetWidth();
		Texture->Desc.Height = FaceImages[0]->GetHeight();
		Texture->Desc.AddressMode = ETC_REPEAT;
		Texture->Desc.SRGB = 0;
		Texture->Desc.Mips = FaceImages[0]->GetMipmapCount();
		Texture->ImageSurfaces.resize(6);
		for (int32 i = 0; i < 6; ++i)
		{
			Texture->ImageSurfaces[i] = FaceImages[i];
		}

		Texture->Name = SrcImage->Name;
		Texture->Path = SrcImage->Path;

		return Texture;
	}
}
