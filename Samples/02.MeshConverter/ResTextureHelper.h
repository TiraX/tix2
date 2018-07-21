/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TResSurfaceData
	{
		int32 W, H;
		TStream Data;
	};
	struct TResTextureDefine
	{
		TString Name;
		TString Path;

		TTextureDesc Desc;
		TVector<TResSurfaceData> Surfaces;
	};

	class TResTextureHelper
	{
	public:
		TResTextureHelper();
		~TResTextureHelper();

		static bool LoadDdsFile(const TString& Filename, TStream& OutStream, TVector<TString>& OutStrings);

		void AddTexture(TResTextureDefine* Texture);
		void OutputTexture(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
		TVector<TResTextureDefine*> Textures;
	};
}