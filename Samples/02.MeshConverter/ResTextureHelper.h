/*
	TiX Engine v2.0 Copyright (C) 2018~2019
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

	class TImage;
	class TResTextureHelper
	{
	public:
		TResTextureHelper();
		~TResTextureHelper();

		static bool LoadTextureFile(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings);
		static TResTextureDefine* LoadDdsFile(const TString& Filename, int32 LodBias);
        static TResTextureDefine* LoadAstcFile(const TString& Filename, int32 LodBias);

		void AddTexture(TResTextureDefine* Texture);
		void OutputTexture(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
		TVector<TResTextureDefine*> Textures;
	};

	void DecodeDXT(TResTextureDefine* Texture, TVector<TImage*>& Images);
}
