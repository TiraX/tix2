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
		int32 RowPitch;
		TStream Data;
	};
	struct TResTextureDefine
	{
		TString Name;
		TString Path;
		int32 LodBias;

		TTextureDesc Desc;
		TVector<TResSurfaceData> Surfaces;
	};

	class TResTextureHelper
	{
	public:
		TResTextureHelper();
		~TResTextureHelper();

		static bool LoadTextureFile(rapidjson::Document& Doc, TStream& OutStream, TVector<TString>& OutStrings);
#if defined (TI_PLATFORM_WIN32)
		static TResTextureDefine* LoadDdsFile(const TString& Filename);
#elif defined (TI_PLATFORM_IOS)
        static TResTextureDefine* LoadAstcFile(const TString& Filename);
#endif

		void AddTexture(TResTextureDefine* Texture);
		void OutputTexture(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
		TVector<TResTextureDefine*> Textures;
	};
}
