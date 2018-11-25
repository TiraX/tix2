/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TResShaderBindingHelper
	{
	public:
		TResShaderBindingHelper();
		~TResShaderBindingHelper();

		static void LoadShaderBinding(rapidjson::Document& Doc, TStream& OutStream, TVector<TString>& OutStrings);

		void OutputShaderBinding(TStream& OutStream, TVector<TString>& OutStrings);
	private:

	private:
		TVector<TBindingParamInfo> Bindings;
	};
}