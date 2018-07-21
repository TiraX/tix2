/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TImage.h"

namespace tix
{
	TTexture::TTexture()
		: TResource(ERES_TEXTURE)
	{}

	TTexture::~TTexture()
	{
		ClearSurfaceData();
	}

	void TTexture::AddSurface(int32 Width, int32 Height, const uint8* Data, int32 RowPitch, int32 DataSize)
	{
		TSurface * Surface = ti_new TSurface;
		uint32 AlignedDataSize = ti_align4(DataSize);
		Surface->Data = ti_new uint8[AlignedDataSize];
		Surface->DataSize = DataSize;
		Surface->RowPitch = RowPitch;
		Surface->Width = Width;
		Surface->Height = Height;
		memcpy(Surface->Data, Data, DataSize);

		Surfaces.push_back(Surface);
	}

	void TTexture::ClearSurfaceData()
	{
		for (TSurface* Surface : Surfaces)
		{
			ti_delete Surface;
		}
		Surfaces.clear();
	}

	void TTexture::InitRenderThreadResource()
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TTextureUpdateFTexture,
			FTexturePtr, Texture_RT, TextureResource,
			TTexturePtr, TextureData, this,
			{
				RHI->UpdateHardwareBuffer(Texture_RT, TextureData);
			});
	}
}