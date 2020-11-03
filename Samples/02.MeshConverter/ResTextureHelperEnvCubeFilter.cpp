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

	SColorf GetPixelFloatWithXWrapLinearMip(TImage* Image, float x, float y, float MipIndex)
	{
		int32 Mip0 = (int32)MipIndex;
		int32 Mip1 = Mip0 + 1;
		
		SColorf C0 = GetPixelFloatWithXWrap(Image, x, y, Mip0);
		SColorf C1 = GetPixelFloatWithXWrap(Image, x, y, Mip1);

		float Frac = MipIndex - Mip0;

		return TMath::Lerp(C0, C1, Frac);
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
		for (int32 MipIndex = 1; MipIndex < TotalMips; ++MipIndex)
		{
			const int32 Extent = ComputeLongLatCubemapExtents(LongLatImage->GetWidth() >> MipIndex);
			const float InvExtent = 1.0f / Extent;
			const int32 W = LongLatImage->GetWidth() >> MipIndex;
			const int32 H = LongLatImage->GetHeight() >> MipIndex;

			float Roughness = ComputeReflectionCaptureRoughnessFromMip(MipIndex, TotalMips - 1);
			const uint32 NumSamples = Roughness < 0.1f ? 32 : 64;

			for (int32 Face = 0; Face < 6; ++Face)
			{
				TImage* FaceImage = FaceImages[Face];

				for (int32 y = 0; y < Extent; ++y)
				{
					for (int32 x = 0; x < Extent; ++x)
					{
						vector3df DirectionWS = ComputeWSCubeDirectionAtTexelCenter(Face, x, y, InvExtent);
						vector2df Coord = DirectionToLongLat(DirectionWS, W, H);

						matrix4 TangentToWorld = GetTangentBasis(DirectionWS);

						SColorf OutColor;

						if (Roughness < 0.01f)
						{
							OutColor = GetPixelFloatWithXWrap(LongLatImage, Coord.X, Coord.Y, MipIndex);
						}
						else
						{
							uint32 CubeSize = 1 << (TotalMips - 1);
							const float SolidAngleTexel = 4 * PI / (6 * CubeSize * CubeSize) * 2;

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

									//L = mul(L, TangentToWorld);
									TangentToWorld.transformVect(L);
									vector2df CoordL = DirectionToLongLat(L, W, H);
									vector4df Sample = GetPixelFloatWithXWrapLinearMip(LongLatImage, CoordL.X, CoordL.Y, Mip);
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

										float ConeAngle = acos(1 - SolidAngleSample / (2 * PI));

										//L = mul(L, TangentToWorld);
										TangentToWorld.transformVect(L);
										vector2df CoordL = DirectionToLongLat(L, W, H);
										vector4df Sample = GetPixelFloatWithXWrapLinearMip(LongLatImage, CoordL.X, CoordL.Y, Mip);
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

						FaceImage->SetPixel(x, y, OutColor, MipIndex);
					}
				}
			}
		}

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

		Texture->Name = SrcImage->Name + "_FilteredCube";
		Texture->Path = SrcImage->Path;

		return Texture;
	}
}
