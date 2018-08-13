/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TVariantValue
	{
		union
		{
			int32 ValueInt;
			float ValueFloat;
			vector3df ValueVec;
			quaternion ValueQuat;
			SColorf ValueClr;
		};
		TString ValueString;

		TVariantValue()
		{
			memset(&ValueClr, 0, sizeof(SColorf));
		}

		TVariantValue& operator = (const TVariantValue& Other)
		{
			ValueClr = Other.ValueClr;
			ValueString = Other.ValueString;
			return *this;
		}
	};
	class TResMaterialInstanceHelper
	{
	public:
		TResMaterialInstanceHelper();
		~TResMaterialInstanceHelper();

		void SetMaterialRes(const TString& MaterialName);

		void AddParameter(const TString& ParamName, int32 Value);
		void AddParameter(const TString& ParamName, float Value);
		void AddParameter(const TString& ParamName, const vector3df& Value);
		void AddParameter(const TString& ParamName, const quaternion& Value);
		void AddParameter(const TString& ParamName, const SColorf& Value);

		void OutputMaterialInstance(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
		TString LinkedMaterial;
		TMap<TString, TVariantValue> Parameters;
	};
}