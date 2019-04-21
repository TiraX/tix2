/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TMIParamValue
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

		TMIParamValue()
		{
			memset(&ValueClr, 0, sizeof(SColorf));
		}

		TMIParamValue(const TMIParamValue& Other)
		{
			memcpy(&ValueClr, &Other.ValueClr, sizeof(SColorf));
			ValueString = Other.ValueString;
		}
	};
	struct TMIParam
	{
		TString ParamName;
		uint8 ParamType;
		TMIParamValue ParamValue;
		vector2di ParamValueSize;

		TMIParam()
			: ParamName("None")
			, ParamType(MIPT_UNKNOWN)
		{
		}

		TMIParam(const TString& InParamName)
			: ParamName(InParamName)
			, ParamType(MIPT_UNKNOWN)
		{
		}
	};

	class TResMaterialInstanceHelper
	{
	public:
		TResMaterialInstanceHelper();
		~TResMaterialInstanceHelper();

		static void LoadMaterialInstance(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings);

		void SetMaterialInstanceName(const TString& InstanceName);
		void SetMaterialRes(const TString& MaterialName);

		void AddParameter(const TString& InParamName, int32 Value);
		void AddParameter(const TString& InParamName, float Value);
		void AddParameter(const TString& InParamName, const vector3df& Value);
		void AddParameter(const TString& InParamName, const quaternion& Value);
		void AddParameter(const TString& InParamName, const SColorf& Value);
		void AddParameter(const TString& InParamName, const TString& Value, const vector2di& Size);

		void OutputMaterialInstance(TStream& OutStream, TVector<TString>& OutStrings);

	private:
		bool IsParamExisted(const TString& InParamName);
	private:
		TString InstanceName;
		TString LinkedMaterial;
		TVector<TMIParam> ValueParameters;
		TVector<TMIParam> TextureParameters;
	};
}