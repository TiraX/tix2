/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TTextureDefine
	{
		TString Name;
		TString Path;
		TTexturePtr TextureRes;
	};

	class TResTextureHelper
	{
	public:
		TResTextureHelper();
		~TResTextureHelper();

		static bool LoadDdsFile(const TString& Filename, TStream& OutStream, TVector<TString>& OutStrings);

		void AddTexture(const TString& Name, const TString& Path, TTexturePtr Texture);
		void OutputTexture(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
		TVector<TTextureDefine> Textures;
	};
}