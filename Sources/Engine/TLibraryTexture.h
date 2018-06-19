/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TLibraryTexture : public TResourceLibrary
	{
	public:
		TLibraryTexture();
		virtual ~TLibraryTexture();

		TI_API virtual void RemoveUnusedResouces() override;

		TI_API virtual bool	IsExist(const TString& texture_name);
		TI_API TTexturePtr GetTexture(const TString& TextureFilename, E_TEXTURE_CLAMP WrapMode = ETC_REPEAT, bool sRGB = true);
		//TI_API TiTextureCubePtr	GetTextureCube(const TString& texture_filename, bool generate_mipmaps = true, E_TEXTURE_CLAMP wraps = ETC_REPEAT, E_TEXTURE_CLAMP wrapt = ETC_REPEAT, E_TEXTURE_CLAMP wrapr = ETC_REPEAT, bool sRGB = true);
		//TI_API TiTexturePtr	CreateTexture(TiImagePtr image, const TString& texture_name, E_TEXTURE_CLAMP wraps = ETC_REPEAT, E_TEXTURE_CLAMP wrapt = ETC_REPEAT);
		//TI_API TiTextureCubePtr	CreateTextureCube(TiImagePtr image, const TString& texture_name, E_TEXTURE_CLAMP wraps = ETC_REPEAT, E_TEXTURE_CLAMP wrapt = ETC_REPEAT, E_TEXTURE_CLAMP wrapr = ETC_REPEAT);

	protected:
		typedef TMap< TString, TTexturePtr >	MapTextures;
		MapTextures Textures;
	};
}