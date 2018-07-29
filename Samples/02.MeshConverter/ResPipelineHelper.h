/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TResPipelineHelper
	{
	public:
		TResPipelineHelper();
		~TResPipelineHelper();

		static bool LoadPipeline(const TString& Filename, TStream& OutStream, TVector<TString>& OutStrings);
		void OutputPipeline(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
	};
}