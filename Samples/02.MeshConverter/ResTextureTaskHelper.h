/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "TImage.h"

namespace tix
{
	template<uint32 bHDR>
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
			int32 H = SrcImage->GetMipmap(SrcMipLevel).H;

			// Down sample to generate mips
			if (bHDR)
			{
				for (int32 y = YStart * 2; y < YEnd * 2; y += 2)
				{
					for (int32 x = 0; x < W; x += 2)
					{
						SColorf c00 = SrcImage->GetPixelFloat(x + 0, y + 0, SrcMipLevel);
						SColorf c10 = SrcImage->GetPixelFloat(x + 1, y + 0, SrcMipLevel);
						SColorf c01 = SrcImage->GetPixelFloat(x + 0, y + 1, SrcMipLevel);
						SColorf c11 = SrcImage->GetPixelFloat(x + 1, y + 1, SrcMipLevel);

						float Rf, Gf, Bf, Af;
						Rf = (float)(c00.R + c10.R + c01.R + c11.R);
						Gf = (float)(c00.G + c10.G + c01.G + c11.G);
						Bf = (float)(c00.B + c10.B + c01.B + c11.B);
						Af = (float)(c00.A + c10.A + c01.A + c11.A);

						// Calc average
						SColorf Target;
						Target.R = (Rf * 0.25f);
						Target.G = (Gf * 0.25f);
						Target.B = (Bf * 0.25f);
						Target.A = (Af * 0.25f);

						SrcImage->SetPixel(x / 2, y / 2, Target, Miplevel);
					}
				}
			}
			else
			{
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
		}
	};
}
