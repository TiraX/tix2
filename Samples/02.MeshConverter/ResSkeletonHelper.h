/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TResSkeletonHelper
	{
	public:
		TResSkeletonHelper();
		~TResSkeletonHelper();

		static void LoadSkeleton(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings);

		void OutputSkeleton(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
	};
}