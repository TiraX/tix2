/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TMaterialParamBinding
	{
		int32 BindingType;
		int32 BindingStage;
		int32 Size;
	};
	class TResMaterialBindingHelper
	{
	public:
		TResMaterialBindingHelper();
		~TResMaterialBindingHelper();

		static void LoadMaterialParameterBinding(rapidjson::Document& Doc, TStream& OutStream, TVector<TString>& OutStrings);

		void OutputMaterialParameterBinding(TStream& OutStream, TVector<TString>& OutStrings);
	private:

	private:
		TVector<TMaterialParamBinding> Bindings;
	};
}