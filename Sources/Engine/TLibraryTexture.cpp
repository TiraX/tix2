/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TLibraryTexture.h"

namespace tix
{
	TLibraryTexture::TLibraryTexture()
	{}

	TLibraryTexture::~TLibraryTexture()
	{
	}

	TTexturePtr TLibraryTexture::GetTexture(const TString& TextureFilename, E_TEXTURE_CLAMP WrapMode, bool sRGB)
	{
		TI_TODO("Remove LibraryTexture later.");
		if (Textures.find(TextureFilename) == Textures.end())
		{
			TImagePtr Image = TImage::LoadImageTix(TextureFilename);
			TTexturePtr Texture = ti_new TTexture;
			Texture->Desc.Format = Image->GetFormat();
			Texture->Desc.Width = Image->GetWidth();
			Texture->Desc.Height = Image->GetHeight();
			Texture->Desc.WrapMode = WrapMode;
			Texture->Desc.SRGB = sRGB;
			Texture->TextureResource = FRHI::Get()->CreateTexture();

			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TTextureUpdateFTexture,
				FTexturePtr, Texture_RT, Texture->TextureResource,
				TImagePtr, Image, Image,
				{
					RHI->UpdateHardwareBuffer(Texture_RT, Image);
				});

			Textures[TextureFilename] = Texture;
			return Texture;
		}
		else
		{
			return Textures[TextureFilename];
		}
	}

	bool TLibraryTexture::IsExist(const TString& texture_name)
	{
		return Textures.find(texture_name) != Textures.end();
	}

	void TLibraryTexture::RemoveUnusedResouces()
	{
		TI_ASSERT(0);
	}
}