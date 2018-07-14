/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TResTextureHelper
	{
	public:
		TResTextureHelper();
		~TResTextureHelper();

		static bool LoadDdsFile(const TString& Filename, TStream& OutStream, TVector<TString>& OutStrings);

		void OutputTexture(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
	};
}