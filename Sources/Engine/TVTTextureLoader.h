/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include "TAssetFileDef.h"

namespace tix
{
	class TVTTextureLoader
	{
	public:
		TVTTextureLoader(const TString& InFileName);

		TTexturePtr LoadTextureWithRegion(int32 Mip, int32 StartX, int32 StartY, int32 EndX, int32 EndY);

	private:
		TFile TextureFile;
		TTextureDesc TextureDesc;
		int32 DataOffset;
	};
}