/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TResAnimSequenceHelper
	{
	public:
		TResAnimSequenceHelper();
		~TResAnimSequenceHelper();

		static void LoadAnimSequence(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings);

		void OutputAnimSequence(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
	};
}